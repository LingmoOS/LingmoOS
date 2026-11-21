// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/window_debug_service.h"
#include "core/main_loop.h"

#include "base/logging.h"
#include <algorithm>
#include <chrono>
#include <utility>

#include <sys/stat.h>

#include "base/json.h"
#include "input/input_manager.h"
#include "scene/surface_node.h"
#include "scene/window_animation.h"
#include "wm/window.h"
#include "wm/window_frame.h"
#include "wm/window_manager.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Session {

namespace {

const base::LogCategory &logs()
{
    return base::LogCategory::defaultCategory();
}

const std::string serviceName = "org.lingmo.WindowDebug1";
const std::string objectPath = "/org/lingmo/WindowDebug1";

using base::json::Value;

Value rectArray(const base::Rect &rect)
{
    return Value::Array {rect.x(), rect.y(), rect.width(), rect.height()};
}

// Members sorted by name at every level, as the output has always had them.
Value sortedMembers(const Value &value)
{
    if (value.isArray()) {
        Value::Array elements;
        for (const Value &element : value.toArray())
            elements.push_back(sortedMembers(element));
        return elements;
    }
    if (!value.isObject())
        return value;
    Value::Object members;
    for (const auto &[key, member] : value.toObject())
        members.emplace_back(key, sortedMembers(member));
    std::stable_sort(members.begin(), members.end(),
                     [](const auto &a, const auto &b) { return a.first < b.first; });
    return Value::object(std::move(members));
}

Value describe(const wm::Window *window)
{
    Value object = Value::object({
            {"id", std::to_string(window->id())},
            {"appId", window->applicationId()},
            {"title", window->title()},
            {"popup", window->isPopup()},
            {"role", wm::toString(window->role())},
            {"state", wm::toString(window->stateType())},
            {"visible", window->isVisible()},
            {"active", window->isActive()},
            {"decorated", window->isDecorated()},
            {"dragPreview", window->isDragPreview()},
            {"bounds", rectArray(window->bounds())},
            {"contentBounds", rectArray(window->contentBounds())},
            {"minimizedGeometry", rectArray(window->minimizedGeometry())},
    });
    if (const auto *animation = dynamic_cast<const scene::WindowAnimation *>(window->node())) {
        object.insert("opacity", double(animation->opacity()));
        object.insert("scale", double(animation->targetPresentationScale()));
    }
    const wm::Insets exterior = window->clientExteriorInsets();
    object.insert("clientExteriorInsets",
                  Value::Array {exterior.left, exterior.top, exterior.right, exterior.bottom});
    const wm::Insets input = window->clientInputInsets();
    object.insert("clientInputInsets",
                  Value::Array {input.left, input.top, input.right, input.bottom});
    if (const wm::WindowFrame *frame = window->frame()) {
        object.insert("titleBar", rectArray(frame->titleBarBounds()));
        Value::Array buttons;
        for (const base::Rect &button : frame->captionButtonBounds())
            buttons.push_back(rectArray(button));
        object.insert("captionButtons", std::move(buttons));
    }
    // What the plane planner and the blur see of the window's surfaces.
    Value::Array surfaces;
    const auto collect = [&](const auto &self, const scene::Node *node) -> void {
        if (!node)
            return;
        if (const auto *surface = dynamic_cast<const scene::SurfaceNode *>(node)) {
            const render::Texture *texture = surface->texture();
            surfaces.push_back(Value::object({
                    {"bounds", rectArray(surface->absoluteGeometry())},
                    {"visible", surface->isEffectivelyVisible()},
                    {"opaque", surface->isOpaque()},
                    {"alpha", double(surface->alpha())},
                    {"blur", surface->hasBackgroundBlur()},
                    {"bufferSize",
                     texture ? Value::Array {texture->size().width(), texture->size().height()}
                             : Value::Array {}},
                    {"scanoutBuffer", surface->scanoutBuffer() != nullptr},
                    {"identityMapping", surface->textureMapping().isIdentity()},
            }));
        }
        for (const scene::Node *child : node->children())
            self(self, child);
    };
    collect(collect, window->node());
    object.insert("surfaces", std::move(surfaces));
    return object;
}
} // namespace

WindowDebugService::WindowDebugService(wm::WindowManager *manager, base::dbus::Connection *bus)
    : m_manager(manager)
    , m_bus(bus ? bus : base::dbus::Connection::session(core::mainLoop()))
{
}

WindowDebugService::~WindowDebugService()
{
    if (m_registered)
        m_bus->releaseName(serviceName);
}

bool WindowDebugService::registerService()
{
    if (m_registered)
        return true;
    if (!m_bus) {
        LM_WARNING(logs()) << "Cannot export the window debug service: no session bus";
        return false;
    }
    auto object = std::make_unique<base::dbus::Object>(m_bus, objectPath, serviceName);
    object->addMethod("ListWindows", [this] { return ListWindows(); });
    object->addMethod("MoveWindow",
                      [this](uint64_t id, int32_t x, int32_t y) { return MoveWindow(id, x, y); },
                      base::dbus::ArgumentNames{{"id", "x", "y"}, {"result"}});
    object->addMethod("SetWindowBounds",
                      [this](uint64_t id, int32_t x, int32_t y, int32_t width, int32_t height) {
                          return SetWindowBounds(id, x, y, width, height);
                      },
                      base::dbus::ArgumentNames{{"id", "x", "y", "width", "height"}, {"result"}});
    object->addMethod("ActivateWindow", [this](uint64_t id) { return ActivateWindow(id); },
                      base::dbus::ArgumentNames{{"id"}, {"result"}});
    object->addMethod("MinimizeWindow", [this](uint64_t id) { return MinimizeWindow(id); },
                      base::dbus::ArgumentNames{{"id"}, {"result"}});
    object->addMethod("SnapWindow",
                      [this](uint64_t id, bool primary) { return SnapWindow(id, primary); },
                      base::dbus::ArgumentNames{{"id", "primary"}, {"result"}});
    object->addMethod("RestoreWindow", [this](uint64_t id) { return RestoreWindow(id); },
                      base::dbus::ArgumentNames{{"id"}, {"result"}});
    object->addMethod("OutputStats", [this] { return OutputStats(); });
    object->addMethod("PointerMotion", [this](double x, double y) { return PointerMotion(x, y); },
                      base::dbus::ArgumentNames{{"x", "y"}, {"result"}});
    object->addMethod("PointerButton", [this](uint32_t button, bool pressed) {
        return PointerButton(button, pressed);
    }, base::dbus::ArgumentNames{{"button", "pressed"}, {"result"}});
    object->addMethod("Key", [this](uint32_t code, bool pressed) { return Key(code, pressed); },
                      base::dbus::ArgumentNames{{"code", "pressed"}, {"result"}});
    if (!object->publish()) {
        LM_WARNING(logs()) << "Cannot export the window debug service";
        return false;
    }
    if (!m_bus->requestName(serviceName)) {
        LM_WARNING(logs()) << "Cannot own the window debug service";
        return false;
    }
    m_object = std::move(object);
    m_registered = true;
    return true;
}

std::string WindowDebugService::ListWindows() const
{
    Value::Array windows;
    for (const wm::Window *window : m_manager->windows())
        windows.push_back(describe(window));
    return sortedMembers(Value(std::move(windows))).serialize();
}

bool WindowDebugService::MoveWindow(uint64_t id, int x, int y)
{
    wm::Window *window = m_manager->windowById(id);
    if (!window)
        return false;
    window->requestBounds(base::Rect(base::Point(x, y), window->bounds().size()));
    return true;
}

bool WindowDebugService::SetWindowBounds(uint64_t id, int x, int y, int width, int height)
{
    wm::Window *window = m_manager->windowById(id);
    if (!window)
        return false;
    window->requestBounds(base::Rect(x, y, width, height));
    return true;
}

bool WindowDebugService::ActivateWindow(uint64_t id)
{
    wm::Window *window = m_manager->windowById(id);
    if (!window)
        return false;
    window->activate();
    return true;
}

bool WindowDebugService::MinimizeWindow(uint64_t id)
{
    wm::Window *window = m_manager->windowById(id);
    if (!window)
        return false;
    window->minimize();
    return true;
}

bool WindowDebugService::SnapWindow(uint64_t id, bool primary)
{
    wm::Window *window = m_manager->windowById(id);
    if (!window)
        return false;
    if (primary)
        window->snapPrimary();
    else
        window->snapSecondary();
    return true;
}

bool WindowDebugService::RestoreWindow(uint64_t id)
{
    wm::Window *window = m_manager->windowById(id);
    if (!window)
        return false;
    window->restore();
    return true;
}

std::vector<std::string> WindowDebugService::OutputStats() const
{
    return m_outputStats ? m_outputStats() : std::vector<std::string>();
}

bool WindowDebugService::acceptsInput() const
{
    if (!m_input)
        return false;
    if (m_inputGate.empty())
        return true;
    // lstat: a symlink to a root-owned file is not the administrator's switch.
    struct stat info = {};
    return lstat(m_inputGate.c_str(), &info) == 0 && S_ISREG(info.st_mode) && info.st_uid == 0
            && !(info.st_mode & (S_IWGRP | S_IWOTH));
}

bool WindowDebugService::PointerMotion(double x, double y)
{
    if (!acceptsInput())
        return false;
    // Where the computer control puts the pointer: logical, whatever the scale.
    m_input->onPointerMotionLogical(std::chrono::steady_clock::now(), base::PointF(x, y));
    return true;
}

bool WindowDebugService::PointerButton(uint32_t button, bool pressed)
{
    if (!acceptsInput())
        return false;
    m_input->onPointerButton(std::chrono::steady_clock::now(), button, pressed);
    return true;
}

bool WindowDebugService::Key(uint32_t code, bool pressed)
{
    if (!acceptsInput())
        return false;
    m_input->onKey(std::chrono::steady_clock::now(), code, pressed);
    return true;
}

} // namespace Lingmo::Session
