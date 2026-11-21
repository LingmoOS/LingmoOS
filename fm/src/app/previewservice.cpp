// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "app/previewservice.h"
#include "core/previewkeys.h"
#include "preview/previewcontroller.h"
#include "preview/previewwindow.h"

#include "base/json.h"

namespace fm {

using base::dbus::MethodCall;

PreviewService::PreviewService(base::dbus::Connection* bus)
    : m_bus(bus)
    , m_object(bus, "/org/lingmo/FilePreview", "org.lingmo.FilePreview")
    , m_controller(std::make_unique<PreviewController>())
{
    m_window = std::make_unique<PreviewWindow>(*m_controller);
    m_window->onNavigate = [this](uint32_t key, uint32_t modifiers) {
        m_object.emitSignal("Navigate", m_source, previewkeys::toWireKey(key), previewkeys::toWireModifiers(modifiers));
    };
    m_subscriptions.push_back(m_controller->closed.connect([this] {
        m_object.emitSignal("VisibilityChanged", false);
        m_object.emitSignal("RestoreFocus", m_source);
        watchOwner();
        if (onVisibilityChanged)
            onVisibilityChanged(false);
    }));

    m_object.addMethod("Toggle", "ss", "", [this](MethodCall& call) {
        std::string source, snapshot;
        call.read(source, snapshot);
        toggle(source, call.message().sender(), snapshot);
        call.reply();
    });
    m_object.addMethod("Activate", "ss", "", [this](MethodCall& call) {
        std::string source, snapshot;
        call.read(source, snapshot);
        if (m_controller->isVisible())
            apply(source, call.message().sender(), snapshot, false);
        call.reply();
    });
    m_object.addMethod("Update", "ss", "", [this](MethodCall& call) {
        std::string source, snapshot;
        call.read(source, snapshot);
        if (m_controller->isVisible() && owns(source, call.message().sender()))
            apply(source, call.message().sender(), snapshot, false);
        call.reply();
    });
    m_object.addMethod("Release", "s", "", [this](MethodCall& call) {
        std::string source;
        call.read(source);
        if (owns(source, call.message().sender())) {
            m_owner.clear();
            m_source.clear();
            watchOwner();
        }
        call.reply();
    });
    m_object.addMethod("Key", "sii", "", [this](MethodCall& call) {
        std::string source;
        int key = 0;
        int modifiers = 0;
        call.read(source, key, modifiers);
        if (owns(source, call.message().sender()) && m_controller->isVisible())
            m_window->sendKey(previewkeys::fromWireKey(key), previewkeys::fromWireModifiers(modifiers));
        call.reply();
    });
    m_object.addMethod("IsVisible", "", "b", [this](MethodCall& call) { call.reply(m_controller->isVisible()); });
    m_object.addSignal("VisibilityChanged", "b");
    m_object.addSignal("Navigate", "sii");
    m_object.addSignal("RestoreFocus", "s");
    m_object.publish();
}

PreviewService::~PreviewService() = default;

bool PreviewService::isVisible() const
{
    return m_controller->isVisible();
}

bool PreviewService::owns(const std::string& source, const std::string& caller) const
{
    return source == m_source && caller == m_owner;
}

// The owner going away gives the preview up, as its Release would.
void PreviewService::watchOwner()
{
    if (!m_controller->isVisible() || m_owner.empty()) {
        m_ownerWatch = {};
        return;
    }
    const std::string owner = m_owner;
    m_ownerWatch = m_bus->watchName(owner, [this, owner](const std::string& current) {
        if (current.empty() && m_owner == owner) {
            m_owner.clear();
            m_source.clear();
        }
    });
}

void PreviewService::toggle(const std::string& source, const std::string& caller, const std::string& snapshot)
{
    if (m_controller->isVisible()) {
        m_source = source;
        m_owner = caller;
        m_controller->close();
        return;
    }
    apply(source, caller, snapshot, true);
}

void PreviewService::apply(const std::string& source, const std::string& caller, const std::string& snapshot,
                           bool activate)
{
    const std::optional<base::json::Value> data = base::json::Value::parse(snapshot);
    if (!data || !data->isObject())
        return;
    std::vector<PreviewItem> items;
    for (const base::json::Value& entry : (*data)["items"].toArray()) {
        PreviewItem item;
        item.url = entry["url"].toString();
        item.name = entry["name"].toString();
        item.icon = entry["icon"].toString();
        item.thumbnail = entry["thumbnail"].toString();
        items.push_back(std::move(item));
    }
    if (items.empty())
        return;
    const bool changedSource = m_source != source || m_owner != caller;
    const bool wasVisible = m_controller->isVisible();
    m_source = source;
    m_owner = caller;
    const int index = int((*data)["currentIndex"].toInt());
    m_controller->present(std::move(items), index, activate, (*data)["ratio"].toDouble());
    if (changedSource)
        m_controller->select(index);
    if (!wasVisible || changedSource)
        watchOwner();
    m_object.emitSignal("VisibilityChanged", true);
    if (!wasVisible && onVisibilityChanged)
        onVisibilityChanged(true);
}

} // namespace fm
