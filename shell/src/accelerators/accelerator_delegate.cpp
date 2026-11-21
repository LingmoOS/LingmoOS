// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "accelerators/accelerator_delegate.h"

#include <chrono>
#include <string>
#include <vector>

#include "base/dbus.h"

namespace Lingmo::Accelerators {

namespace {

// Calls without waiting for the answer.
template<typename... Args>
void call(const char *service, const char *path, const char *method, const Args &...arguments)
{
    if (base::dbus::Connection *bus = base::dbus::Connection::session())
        base::dbus::Proxy(bus, service, path, service).send(method, arguments...);
}

template<typename... Args>
void callScreenshot(const char *method, const Args &...arguments)
{
    call("org.lingmo.Screenshot", "/Screenshot", method, arguments...);
}

void callOsd(const char *method)
{
    call("org.lingmo.Osd", "/Osd", method);
}

// MPRIS players register a bus name of their own, so the session's are found
// by prefix. A player that is playing answers before one that is only open:
// a paused browser tab should not take the key from the music that is on.
constexpr const char *kMprisPrefix = "org.mpris.MediaPlayer2.";

std::string activePlayer(base::dbus::Connection *bus)
{
    const base::dbus::Message list = bus->newMethodCall(
            "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", "ListNames");

    // The bus is local and answers at once; the timeout only bounds one that
    // has stopped answering, so that the session's input is not held by it.
    std::vector<std::string> names;
    if (!bus->call(list, std::chrono::milliseconds(500)).read(names))
        return {};

    std::string idle;
    for (const std::string &name : names) {
        if (name.rfind(kMprisPrefix, 0) != 0)
            continue;
        const base::dbus::Proxy player(bus, name, "/org/mpris/MediaPlayer2",
                                       "org.mpris.MediaPlayer2.Player");
        if (player.property<std::string>("PlaybackStatus").value_or("") == "Playing")
            return name;
        if (idle.empty())
            idle = name;
    }
    return idle;
}

void callPlayer(const char *method)
{
    base::dbus::Connection *bus = base::dbus::Connection::session();
    if (!bus)
        return;
    const std::string player = activePlayer(bus);
    if (player.empty())
        return;
    base::dbus::Proxy(bus, player, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player")
            .send(method);
}

} // namespace

void DBusAcceleratorDelegate::toggleAppList()
{
    if (launcherToggle) {
        launcherToggle();
        return;
    }
    call("org.lingmo.Launcher", "/Launcher", "toggle");
}

void DBusAcceleratorDelegate::toggleSearch()
{
    // The session bus activates lingmo-search on demand. Forward a shell-issued
    // token so the search field can receive focus without a focus-stealing bypass.
    call("org.lingmo.Search", "/org/lingmo/Search", "toggle",
         searchActivationToken ? searchActivationToken() : std::string());
}

void DBusAcceleratorDelegate::toggleScreenKeyboard()
{
    call("org.lingmo.ScreenKeyboard", "/org/lingmo/ScreenKeyboard", "toggle");
}

void DBusAcceleratorDelegate::takeScreenshot(bool toClipboard)
{
    callScreenshot("takeScreenshot", toClipboard);
}

void DBusAcceleratorDelegate::takePartialScreenshot(bool toClipboard)
{
    callScreenshot("takePartialScreenshot", toClipboard);
}

void DBusAcceleratorDelegate::startCaptureMode()
{
    callScreenshot("startCaptureMode");
}

void DBusAcceleratorDelegate::stopScreenRecording()
{
    callScreenshot("stopRecording");
}

void DBusAcceleratorDelegate::toggleClipboardHistory()
{
    call("org.lingmo.ClipboardHistory", "/ClipboardHistory", "toggle");
}

void DBusAcceleratorDelegate::volumeDown()
{
    callOsd("volumeDown");
}

void DBusAcceleratorDelegate::volumeUp()
{
    callOsd("volumeUp");
}

void DBusAcceleratorDelegate::toggleMute()
{
    callOsd("toggleMute");
}

void DBusAcceleratorDelegate::toggleMicrophoneMute()
{
    callOsd("toggleMicrophoneMute");
}

void DBusAcceleratorDelegate::brightnessDown()
{
    callOsd("brightnessDown");
}

void DBusAcceleratorDelegate::brightnessUp()
{
    callOsd("brightnessUp");
}

void DBusAcceleratorDelegate::keyboardBrightnessDown()
{
    callOsd("keyboardBrightnessDown");
}

void DBusAcceleratorDelegate::keyboardBrightnessUp()
{
    callOsd("keyboardBrightnessUp");
}

void DBusAcceleratorDelegate::mediaPlayPause()
{
    callPlayer("PlayPause");
}

void DBusAcceleratorDelegate::mediaNext()
{
    callPlayer("Next");
}

void DBusAcceleratorDelegate::mediaPrevious()
{
    callPlayer("Previous");
}

void DBusAcceleratorDelegate::mediaStop()
{
    callPlayer("Stop");
}

} // namespace Lingmo::Accelerators
