// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "session/applications_service.h"
#include "core/main_loop.h"

#include "base/logging.h"
#include <utility>

#include "wm/quit_all.h"

namespace Lingmo::Session {

namespace {

const base::LogCategory &logs()
{
    return base::LogCategory::defaultCategory();
}

const std::string serviceName = "org.lingmo.Applications1";
const std::string objectPath = "/org/lingmo/Applications1";

std::string outcomeName(wm::QuitAll::Outcome outcome)
{
    switch (outcome) {
    case wm::QuitAll::Outcome::Quit:
        return "quit";
    case wm::QuitAll::Outcome::Cancelled:
        return "cancelled";
    case wm::QuitAll::Outcome::Unresponsive:
        return "unresponsive";
    }
    return {};
}
} // namespace

ApplicationsService::ApplicationsService(wm::QuitAll *quitAll,
                                         std::function<void(bool)> setEnding,
                                         base::dbus::Connection *bus)
    : m_quitAll(quitAll)
    , m_setEnding(std::move(setEnding))
    , m_bus(bus ? bus : base::dbus::Connection::session(core::mainLoop()))
{
}

ApplicationsService::~ApplicationsService()
{
    for (auto &request : m_pending)
        request.replyError("org.freedesktop.DBus.Error.Failed", "Compositor is shutting down");
    if (m_registered)
        m_bus->releaseName(serviceName);
}

bool ApplicationsService::registerService()
{
    if (m_registered)
        return true;
    if (!m_bus) {
        LM_WARNING(logs()) << "Cannot export the applications service: no session bus";
        return false;
    }
    auto object = std::make_unique<base::dbus::Object>(m_bus, objectPath, serviceName);
    // sd-bus names every argument or none, so the return value is "result".
    object->addMethod("QuitAll", "", "ss", [this](base::dbus::MethodCall &call) { quitAll(call); },
                      base::dbus::ArgumentNames{{}, {"result", "applicationId"}});
    if (!object->publish()) {
        LM_WARNING(logs()) << "Cannot export the applications service";
        return false;
    }
    if (!m_bus->requestName(serviceName)) {
        LM_WARNING(logs()) << "Cannot own the applications service";
        return false;
    }
    m_object = std::move(object);
    m_registered = true;
    return true;
}

void ApplicationsService::quitAll(base::dbus::MethodCall &call)
{
    m_pending.push_back(call);
    if (m_quitAll->isRunning())
        return;

    if (m_setEnding)
        m_setEnding(true);
    m_quitAll->start([this](const wm::QuitAll::Result &result) {
        if (result.outcome != wm::QuitAll::Outcome::Quit && m_setEnding)
            m_setEnding(false);
        auto pending = std::exchange(m_pending, {});
        for (auto &request : pending)
            request.reply(outcomeName(result.outcome), result.applicationId);
    });
}

} // namespace Lingmo::Session
