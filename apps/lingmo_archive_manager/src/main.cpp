// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "archivewindow.h"
#include "base/dbus.h"
#include "base/i18n.h"
#include "base/url.h"
#include "ui/application.h"
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>
int main(int argc, char** argv) {
    std::vector<std::string> paths; bool createMode = false, options = true;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (options && arg == "--") { options = false; continue; }
        if (options && (arg == "--help" || arg == "-h")) {
            std::puts("Usage: lingmo-archive-manager [--create] [--] [FILES...]\nOpen one archive, or create a new archive from files and folders."); return 0;
        }
        if (options && arg == "--version") { std::puts("Lingmo Archive Manager 1.0.0"); return 0; }
        if (options && arg == "--create") { createMode = true; continue; }
        if (options && arg.starts_with('-')) { std::fprintf(stderr, "Unknown option: %s\n", arg.c_str()); return 1; }
        std::string path = arg;
        if (arg.find("://") != std::string::npos) {
            auto local = Lingmo::base::url::toLocalFile(arg);
            if (!local) { std::fprintf(stderr, "Only local files are supported\n"); return 1; }
            path = *local;
        }
        std::error_code error;
        path = std::filesystem::absolute(path, error).lexically_normal().string();
        if (error) { std::fprintf(stderr, "Invalid path\n"); return 1; }
        paths.push_back(std::move(path));
    }
    Lingmo::ui::Application app(argc, argv); app.setApplicationId("lingmo-archive-manager");
    Lingmo::i18n::setDomain("lingmo-archive-manager");
    auto* bus = app.sessionBus();
    constexpr auto service = "org.lingmo.ArchiveManager";
    if (bus && !bus->requestName(service)) {
        Lingmo::base::dbus::Proxy existing(bus, service, "/ArchiveManager", service);
        existing.setTimeout(std::chrono::seconds(5));
        const auto reply = existing.call("open", paths, createMode, app.takeActivationToken());
        if (reply.isError() || !reply.value<bool>().value_or(false)) { std::fprintf(stderr, "Archive Manager is busy or unavailable\n"); return 1; }
        return 0;
    }
    ArchiveManager::ArchiveWindow window;
    Lingmo::base::dbus::Object object(bus, "/ArchiveManager", service);
    object.addMethod("open", [&window](const std::vector<std::string>& paths, bool create, const std::string& token) {
        window.window().activate(token); return window.openArguments(paths, create);
    });
    if (bus) object.publish();
    window.show(); if (createMode) window.newArchive(); window.openArguments(paths, createMode);
    return app.run();
}
