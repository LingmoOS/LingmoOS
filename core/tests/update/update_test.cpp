// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "updatemanager.h"
#include "lingmo/update/update_status.h"
#include "base/event_loop.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <poll.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
using namespace Lingmo;
using namespace std::chrono_literals;
void require(bool ok, const char* message) { if (!ok) { std::cerr << message << '\n'; std::exit(1); } }
void wait(base::EventLoop& loop, const std::function<bool()>& ready) {
    auto end = std::chrono::steady_clock::now() + 5s;
    while (!ready() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 5); loop.dispatch(); }
    require(ready(), "Timed out waiting for update state");
}
void write(const std::string& path, const std::string& text, bool executable = false) {
    std::ofstream file(path); file << text; file.close(); if (executable) ::chmod(path.c_str(), 0700);
}
int main() {
    std::string temp = "/tmp/lingmo-update-test-XXXXXX"; require(::mkdtemp(temp.data()), "temporary directory");
    struct Cleanup { std::string path; ~Cleanup() { std::filesystem::remove_all(path); } } cleanup{temp};
    // PATH contains only test doubles. No real curl, apt or pkexec can execute.
    ::setenv("PATH", temp.c_str(), 1);
    const auto mode = [&](const std::string& value) { write(temp + "/mode", value + "\n"); };
    const std::string prefix = "#!/bin/sh\nread mode < '" + temp + "/mode'\n";
    const auto curlScript = [&] {
        write(temp + "/curl", prefix +
            "echo \"$7\" >> '" + temp + "/curl-urls'\n" +
            "case \"$mode\" in\n"
            " api-fail) echo 'TLS connection failed' >&2; exit 60;;\n"
            " invalid-api) echo '<html>unavailable</html>'; exit 0;;\n"
            " system-upgrade) echo '{\"success\":true,\"updateAvailable\":true,\"latest\":{\"version\":\"5.0\",\"buildVersion\":\"26R01\",\"buildCode\":999999999,\"releaseDate\":\"2026-11-03\",\"mandatory\":false},\"summary\":\"<h2>What is New</h2><ul><li>Test</li></ul>\",\"releaseNotesUrl\":\"/releases/26R01\"}'; exit 0;;\n"
            "esac\necho '{\"success\":true,\"updateAvailable\":false}'\n", true);
    };
    write(temp + "/pkexec", prefix +
        "echo \"$2\" >> '" + temp + "/apt-actions'\n" +
        "if [ \"$2\" = update ]; then\n"
        " if [ \"$mode\" = refresh-fail ]; then echo 'Authorization denied' >&2; exit 126; fi\n"
        " /bin/sleep 0.05; exit 0\nfi\n"
        "if [ \"$mode\" = upgrade-fail ]; then echo 'Package lock unavailable' >&2; exit 100; fi\n"
        "/bin/sleep 0.05\n"
        "echo 'dlstatus:pkg:40:Downloading'\necho 'pmstatus:pkg:100:Installed'\n", true);
    write(temp + "/apt", prefix +
        "if [ \"$mode\" = list-fail ]; then echo 'Package cache unreadable' >&2; exit 100; fi\n"
        "echo 'sample/stable 2.0 amd64 [upgradable from: 1.0]'\n", true);
    base::EventLoop loop;
    auto* bus = base::dbus::Connection::session(&loop); require(bus, "session bus");

    // The update service's notifications, from progress to the restart prompt.
    std::vector<std::string> summaries;
    std::vector<int32_t> values;
    std::vector<std::vector<std::string>> actions;
    base::dbus::Object notifier(bus, "/org/freedesktop/Notifications",
                                "org.freedesktop.Notifications");
    notifier.addMethod("Notify", "susssasa{sv}i", "u", [&](base::dbus::MethodCall& call) {
        std::string appName, icon, summary, body;
        uint32_t replaces = 0;
        std::vector<std::string> callActions;
        base::dbus::VariantMap hints;
        int32_t timeout = -1;
        if (!call.read(appName, replaces, icon, summary, body, callActions, hints, timeout)) {
            call.replyError("org.freedesktop.DBus.Error.InvalidArgs", "Unexpected arguments");
            return;
        }
        summaries.push_back(summary);
        const auto value = hints.find("value");
        values.push_back(value == hints.end() ? -1 : value->second.value<int32_t>(-1));
        actions.push_back(callActions);
        call.reply(uint32_t(1));
    });
    require(notifier.publish() && bus->requestName("org.freedesktop.Notifications"),
            "notification service");

    update::UpdateStatus status; // Before the service exists: no synchronous self-call.
    base::Settings settings(temp + "/settings");
    auto manager = std::make_unique<UpdateManager>(settings, bus);
    require(settings.contains("Update/Channel"), "channel seeded from os-release");
    require(manager->publish() && bus->requestName("org.lingmo.Services"), "publish update service");
    wait(loop, [&] { return status.isAvailable(); });
    const auto aptActions = [&] {
        std::ifstream file(temp + "/apt-actions");
        std::vector<std::string> actions;
        std::string line;
        while (std::getline(file, line))
            actions.push_back(line);
        return actions;
    };
    const auto curlUrls = [&] {
        std::ifstream file(temp + "/curl-urls");
        std::vector<std::string> urls;
        std::string line;
        while (std::getline(file, line))
            urls.push_back(line);
        return urls;
    };
    auto check = [&](const std::string& scenario, update::State expected) {
        mode(scenario); curlScript();
        bool observedChecking = false;
        auto changed = status.changed.connect([&] { if (status.state() == update::State::Checking) observedChecking = true; });
        status.checkForUpdate(); require(status.isRequestPending(), "command acknowledgement pending");
        wait(loop, [&] { return observedChecking && !status.isRequestPending() && status.state() == expected; });
    };
    check("api-fail", update::State::Error);
    require(status.errorLog().find("TLS connection failed") != std::string::npos, "curl diagnostics visible");
    check("success", update::State::SoftwareUpdate);
    require(status.errorLog().empty() && status.upgradableCount() == 1, "retry clears error and finds update");
    const auto urls = curlUrls();
    require(std::any_of(urls.begin(), urls.end(), [](const std::string& url) {
                return url.find("/api/v1/update/check") != std::string::npos
                    && url.find("build=") != std::string::npos
                    && url.find("locale=") != std::string::npos;
            }),
            "API uses the build-aware endpoint");
    check("refresh-fail", update::State::Error);
    require(status.errorLog().find("Authorization denied") != std::string::npos, "APT refresh failure visible");
    check("list-fail", update::State::Error);
    require(status.errorLog().find("Package cache unreadable") != std::string::npos, "nonzero apt exit not up-to-date");
    check("invalid-api", update::State::Error);
    require(!status.errorLog().empty(), "invalid API response has details");
    check("system-upgrade", update::State::SystemUpgrade);
    require(status.upgradableCount() == 1, "system upgrade check reaches APT");
    require(status.newVersion() == "5.0", "API version reaches the client");
    require(status.releaseDate() == "2026-11-03", "API release date reaches the client");
    require(status.changelogText().find("What is New") != std::string::npos,
            "API summary reaches the client");
    mode("upgrade-fail"); status.startUpgrade();
    wait(loop, [&] { return status.state() == update::State::Error && !status.isRequestPending(); });
    require(status.errorLog().find("Package lock unavailable") != std::string::npos, "upgrade stderr visible");
    const auto systemActions = aptActions();
    require(std::find(systemActions.begin(), systemActions.end(), "dist-upgrade") != systemActions.end(),
            "system upgrade uses dist-upgrade");
    wait(loop, [&] { return !summaries.empty() && summaries.back() == "Update failed"; });
    check("success", update::State::SoftwareUpdate);
    status.startUpgrade(); wait(loop, [&] { return status.state() == update::State::RestartRequired && !status.isRequestPending(); });
    wait(loop, [&] { return !summaries.empty() && summaries.back() == "Restart required"; });
    require(status.errorLog().empty(), "upgrade retry succeeds");
    require(values.back() == -1, "restart notification has no progress bar");
    require(actions.back().size() == 2 && actions.back()[0] == "restart",
            "restart notification offers a restart action");
    require(std::find(summaries.begin(), summaries.end(), "Updating system") != summaries.end(),
            "progress notification");
    require(std::any_of(values.begin(), values.end(), [](int value) { return value >= 0; }),
            "progress notification carries a progress bar");
    const auto softwareActions = aptActions();
    require(std::find(softwareActions.begin(), softwareActions.end(), "upgrade") != softwareActions.end(),
            "software update uses upgrade");

    // A check must not clear the restart prompt; only a new session does.
    bool checking = false;
    [[maybe_unused]] auto observed =
            status.changed.connect([&] { checking |= status.state() == update::State::Checking; });
    status.checkForUpdate();
    for (auto end = std::chrono::steady_clock::now() + 200ms; std::chrono::steady_clock::now() < end;) {
        pollfd fd{loop.fd(), POLLIN, 0};
        ::poll(&fd, 1, 5);
        loop.dispatch();
    }
    require(!checking && status.state() == update::State::RestartRequired,
            "check does not clear the restart prompt");
    wait(loop, [&] { return !status.isRequestPending(); });
    manager.reset();
    // Service name remains, but method calls now fail: surface the D-Bus error.
    status.checkForUpdate(); wait(loop, [&] { return !status.isRequestPending(); });
    require(!status.requestError().empty(), "D-Bus method errors are not silent");
    bus->releaseName("org.lingmo.Services"); wait(loop, [&] { return !status.isAvailable(); });
    std::cout << "Update retry integration passed\n";
}
