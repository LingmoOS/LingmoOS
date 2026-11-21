// SPDX-License-Identifier: GPL-3.0-or-later
#include "sound/soundservice.h"
#include "base/event_loop.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include <stdexcept>
#include <unistd.h>

namespace fs = std::filesystem;
namespace dbus = Lingmo::base::dbus;
void check(bool condition, const char *message) { if (!condition) throw std::runtime_error(message); }
int main()
{
    char temporary[] = "/tmp/lingmo-sound-XXXXXX";
    const char *directory = mkdtemp(temporary);
    if (!directory) return 1;
    const fs::path root(directory);
    try {
        Lingmo::base::EventLoop loop;
        auto bus = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        check(bool(bus), "private bus");
        Lingmo::base::Settings settings((root / "services.conf").string());
        fs::create_directories(root / "sounds/freedesktop/stereo");
        fs::create_directories(root / "sounds/custom/stereo");
        std::ofstream(root / "sounds/freedesktop/stereo/message-new-instant.oga") << "fixture";
        std::ofstream(root / "sounds/freedesktop/stereo/camera-shutter.oga") << "fixture";
        std::ofstream(root / "sounds/custom/stereo/message-new-instant.wav") << "fixture";
        std::vector<std::string> played;
        SoundService service(settings, bus.get(), {root.string()}, [&](const std::string &file) {
            played.push_back(file);
            return true;
        });
        check(service.publish(), "publish");
        check(bus->requestName("org.lingmo.Services"), "own name");
        check(service.playEvent("screen-capture"), "shutter fallback");
        check(played.back().ends_with("camera-shutter.oga"), "shutter filename");
        check(!service.playEvent("screen-capture"), "rate limit");
        check(service.soundFile("../secret").empty(), "path traversal");
        check(service.soundFile("/tmp/sound.wav").empty(), "absolute path");
        check(service.soundFile("missing").empty(), "missing sound");
        service.setTheme("custom");
        check(service.soundFile("message-new-instant").ends_with("custom/stereo/message-new-instant.wav"), "theme precedence");
        check(service.soundFile("screen-capture").ends_with("freedesktop/stereo/camera-shutter.oga"), "theme fallback");
        service.setTheme("../bad");
        check(service.theme() == "custom", "invalid theme");
        service.setEnabled(false);
        check(!service.playEvent("message-new-instant"), "disabled playback");
        Lingmo::base::Settings saved(settings.path());
        check(!saved.boolValue("Sound/Enabled", true), "saved switch");
        check(saved.stringValue("Sound/Theme") == "custom", "saved theme");

        auto client = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        dbus::Proxy proxy(client.get(), "org.lingmo.Services", "/org/lingmo/Services/Sound", "org.lingmo.Services.Sound");
        bool replied = false;
        auto call = proxy.callAsync("playEvent", [&](const dbus::Reply &reply) {
            check(!reply.isError(), "D-Bus call");
            check(reply.value<bool>() == false, "disabled D-Bus response");
            replied = true;
            loop.quit();
        }, std::string("message-new-instant"));
        Lingmo::base::Timer timeout;
        timeout.setSingleShot(true);
        timeout.onTimeout = [&] { loop.quit(1); };
        timeout.start(std::chrono::seconds(3));
        loop.run();
        check(replied, "D-Bus timeout");
        timeout.stop();
        replied = false;
        call = proxy.callAsync("setEnabled", [&](const dbus::Reply &reply) {
            check(!reply.isError(), "D-Bus setter");
            replied = true;
            loop.quit();
        }, true);
        timeout.start(std::chrono::seconds(3));
        loop.run();
        check(replied && service.enabled(), "D-Bus enables sound");
        timeout.stop();
        replied = false;
        call = proxy.getPropertyAsync("enabled", [&](const dbus::Reply &reply) {
            const auto value = reply.value<dbus::Variant>();
            check(value && value->get<bool>() == true, "D-Bus enabled property");
            replied = true;
            loop.quit();
        });
        timeout.start(std::chrono::seconds(3));
        loop.run();
        check(replied, "D-Bus property timeout");
        timeout.stop();
        // Exercise the owned player without sending anything to speakers.
        // Its executable records the argument, then stays alive until muted.
        fs::create_directories(root / "bin");
        const fs::path executable = root / "bin/pw-play";
        std::ofstream(executable) << "#!/bin/sh\nprintf '%s\\n' \"$$\" \"$1\" > \"$LINGMO_SOUND_TEST_OUTPUT\"\nexec /bin/sleep 30\n";
        fs::permissions(executable, fs::perms::owner_all);
        const std::string oldPath = std::getenv("PATH") ? std::getenv("PATH") : "";
        setenv("PATH", (root / "bin").c_str(), 1);
        setenv("LINGMO_SOUND_TEST_OUTPUT", (root / "player-output").c_str(), 1);
        SoundService player(settings, nullptr, {root.string()});
        check(player.playEvent("message-new-instant"), "start owned player");
        timeout.onTimeout = [&] { loop.quit(); };
        timeout.start(std::chrono::milliseconds(150));
        loop.run();
        std::ifstream output(root / "player-output");
        int pid = 0;
        std::string file;
        output >> pid >> file;
        check(pid > 0 && file.ends_with("message-new-instant.wav"), "player arguments");
        check(!player.playEvent("screen-capture"), "only one concurrent player");
        player.setEnabled(false);
        timeout.start(std::chrono::milliseconds(100));
        loop.run();
        check(kill(pid, 0) == -1 && errno == ESRCH, "mute stops and reaps player");
        setenv("PATH", oldPath.c_str(), 1);
        unsetenv("LINGMO_SOUND_TEST_OUTPUT");
        std::cout << "Sound service tests passed\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        fs::remove_all(root);
        return 1;
    }
    fs::remove_all(root);
}
