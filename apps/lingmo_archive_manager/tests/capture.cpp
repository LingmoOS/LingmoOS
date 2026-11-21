// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "archivewindow.h"
#include "base/event_loop.h"
#include "ui/application.h"
#include "ui/theme.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <poll.h>
using namespace ArchiveManager;
using namespace Lingmo::ui;
void wait(ArchiveWindow& window) {
    auto& loop = Application::instance()->loop(); auto end = std::chrono::steady_clock::now() + std::chrono::seconds(20);
    while (window.busy() && std::chrono::steady_clock::now() < end) { pollfd fd{loop.fd(), POLLIN, 0}; ::poll(&fd, 1, 10); loop.dispatch(); }
}
int main(int argc, char** argv) {
    if (argc != 2) return 1;
    std::filesystem::create_directories(argv[1]);
    std::string path = "/tmp/lingmo-archive-capture-XXXXXX"; if (!::mkdtemp(path.data())) return 1;
    struct Temp { std::filesystem::path path; ~Temp() { std::filesystem::remove_all(path); } } temp{path};
    std::filesystem::create_directories(path + "/Project/Assets"); std::filesystem::create_directories(path + "/Project/Documents");
    std::ofstream(path + "/Project/README.md") << "Welcome to Lingmo Archive Manager";
    std::ofstream(path + "/Project/Documents/Design notes.txt") << std::string(2048, 'd');
    std::ofstream(path + "/Project/Assets/Wallpaper.txt") << std::string(12000, 'a');
    std::ofstream(path + "/Project/Changelog.txt") << std::string(640, 'c');
    auto made = create({path + "/Project"}, path + "/Project.zip", Format::Zip); if (!made.ok()) return 2;
    Application app(argc, argv, Application::Platform::Headless);
    for (bool dark : {false, true}) for (double scale : {1.0, 1.5}) {
        Theme::setCurrent(dark ? Theme::dark() : Theme::light()); ArchiveWindow window; window.window().setScale(scale); window.show();
        const std::string suffix = std::string(dark ? "dark" : "light") + (scale == 1 ? "-100" : "-150");
        window.openArchive(made.output); wait(window); if (window.busy()) return 3; window.navigate("Project");
        if (!window.window().grab().savePng(std::string(argv[1]) + "/archive-" + suffix + ".png")) return 4;
        window.newArchive(); window.addSources({path + "/Project"});
        if (!window.window().grab().savePng(std::string(argv[1]) + "/create-" + suffix + ".png")) return 4;
    }
}
