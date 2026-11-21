// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "privacyservice.h"

#include "base/logging.h"
#include "common/detached_call.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <map>
#include <tuple>
#include <unistd.h>

namespace dbus = Lingmo::Base::dbus;

static const Lingmo::Base::LogCategory kLog("lingmo.privacy");

static constexpr char kAction[] = "org.lingmo.privacy.switch-devices";
static const char *const kCamera = "camera";
static const char *const kMicrophone = "microphone";

// polkit's Subject: a kind and its details.
using PolkitSubject = std::tuple<std::string, dbus::VariantMap>;

PrivacyService::PrivacyService(const std::string &stateDirectory, std::chrono::milliseconds idleTimeout)
    : m_stateDirectory(stateDirectory)
{
    std::error_code error;
    std::filesystem::create_directories(m_stateDirectory, error);

    // Again at start, for devices that were present before the udev rules.
    if (!cameraEnabled())
        m_cameras.setAuthorized(false);

    if (!microphoneEnabled())
        m_captureSwitches.setLocked(true);

    m_idleTimer.setSingleShot(true);
    m_idleTimer.setInterval(idleTimeout);
    m_idleTimer.onTimeout = [this] {
        // An administrator may still be typing a password for a call.
        if (!m_captureSwitches.isLocked() && !m_pendingAuthorizations && onIdle)
            onIdle();
    };
    m_idleTimer.start();
}

bool PrivacyService::exportTo(dbus::Connection *bus)
{
    m_bus = bus;
    m_object = std::make_unique<dbus::Object>(bus, "/org/lingmo/Privacy", "org.lingmo.Privacy");
    m_object->addProperty("cameraEnabled", [this] { return cameraEnabled(); });
    m_object->addProperty("microphoneEnabled", [this] { return microphoneEnabled(); });
    m_object->addMethod(
            "setCameraEnabled", "b", "",
            [this](dbus::MethodCall &call) {
                bool enabled = false;
                call.read(enabled);
                authorize(call, [this, enabled] {
                    applyCamera(enabled);
                });
            },
            {{"enabled"}, {}});
    m_object->addMethod(
            "setMicrophoneEnabled", "b", "",
            [this](dbus::MethodCall &call) {
                bool enabled = false;
                call.read(enabled);
                authorize(call, [this, enabled] {
                    applyMicrophone(enabled);
                });
            },
            {{"enabled"}, {}});
    m_object->addSignal("cameraEnabledChanged", "b", {"enabled"});
    m_object->addSignal("microphoneEnabledChanged", "b", {"enabled"});
    return m_object->publish();
}

bool PrivacyService::cameraEnabled() const
{
    return ::access(flagPath(kCamera).c_str(), F_OK) != 0;
}

bool PrivacyService::microphoneEnabled() const
{
    return ::access(flagPath(kMicrophone).c_str(), F_OK) != 0;
}

void PrivacyService::setCameraEnabled(bool enabled)
{
    m_idleTimer.start();
    applyCamera(enabled);
}

void PrivacyService::setMicrophoneEnabled(bool enabled)
{
    m_idleTimer.start();
    applyMicrophone(enabled);
}

void PrivacyService::authorize(dbus::MethodCall &call, std::function<void()> apply)
{
    m_idleTimer.start();

    dbus::MethodCall request = call;
    dbus::Message check = m_bus->newMethodCall("org.freedesktop.PolicyKit1", "/org/freedesktop/PolicyKit1/Authority",
                                               "org.freedesktop.PolicyKit1.Authority", "CheckAuthorization");
    const PolkitSubject subject {"system-bus-name", {{"name", dbus::Variant(call.message().sender())}}};
    // 1: AllowUserInteraction, should an administrator require a password.
    check.append(subject, std::string(kAction), std::map<std::string, std::string>(), uint32_t(1), std::string());

    // No timeout: an administrator may take a while over the password.
    ++m_pendingAuthorizations;
    Lingmo::Core::callDetached(
            m_bus, check,
            [this, request, apply](const dbus::Reply &reply) mutable {
                --m_pendingAuthorizations;
                m_idleTimer.start();

                bool authorized = false;
                std::tuple<bool, bool, std::map<std::string, std::string>> result;
                if (reply.read(result))
                    authorized = std::get<0>(result);
                else
                    LM_WARNING(kLog) << "lingmo-privacy: polkit check failed:" << reply.error().message;

                if (!authorized) {
                    request.replyError("org.freedesktop.DBus.Error.AccessDenied",
                                       "Not authorized to turn the camera or microphone on or off");
                } else {
                    apply();
                    request.reply();
                }
            },
            std::chrono::hours(24));
}

void PrivacyService::applyCamera(bool enabled)
{
    const bool changed = enabled != cameraEnabled();

    // The flag before the interfaces, so a camera plugged in meanwhile follows it.
    setFlag(kCamera, !enabled);
    m_cameras.setAuthorized(enabled);

    if (changed && m_object)
        m_object->emitSignal("cameraEnabledChanged", enabled);
}

void PrivacyService::applyMicrophone(bool enabled)
{
    const bool changed = enabled != microphoneEnabled();

    setFlag(kMicrophone, !enabled);
    m_captureSwitches.setLocked(!enabled);

    if (changed && m_object)
        m_object->emitSignal("microphoneEnabledChanged", enabled);
}

std::string PrivacyService::flagPath(const char *device) const
{
    return m_stateDirectory + '/' + device + "-off";
}

void PrivacyService::setFlag(const char *device, bool off)
{
    const std::string path = flagPath(device);

    if (!off) {
        ::unlink(path.c_str());
        return;
    }

    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (fd < 0) {
        LM_WARNING(kLog) << "lingmo-privacy: cannot write" << path << std::strerror(errno);
        return;
    }
    ::close(fd);
}
