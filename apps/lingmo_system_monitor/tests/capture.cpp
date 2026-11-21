// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Renders the window headless, every page, with its layout:
//   system-monitor-capture DIR [WIDTH HEIGHT SCALE [dark]]
#include "icons.h"
#include "mainwindow.h"
#include "monitor.h"

#include "base/event_loop.h"
#include "base/timer.h"
#include "ui/application.h"
#include "ui/icon.h"
#include "ui/theme.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

using namespace Lingmo;
using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s DIR [WIDTH HEIGHT SCALE [dark]]\n", argv[0]);
        return 1;
    }
    const std::string directory = argv[1];
    const double width = argc > 2 ? std::atof(argv[2]) : 840;
    const double height = argc > 3 ? std::atof(argv[3]) : 600;
    const double scale = argc > 4 ? std::atof(argv[4]) : 1.5;
    const bool dark = argc > 5 && std::string(argv[5]) == "dark";
    std::filesystem::create_directories(directory);

    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    for (const auto& [name, svg] : embeddedIcons())
        ui::Icon::registerGlyph(std::string(name), std::string(svg));
    ui::Theme::setCurrent(dark ? ui::Theme::dark() : ui::Theme::light());
    Monitor monitor;
    MainWindow window(monitor);
    window.window().setScale(scale);
    window.window().resize({width, height});

    base::Timer poll;
    poll.onTimeout = [&] {
        if (monitor.ready())
            app.quit();
    };
    poll.start(20ms);
    app.run();
    // A few samples, so the charts have history.
    base::Timer wait;
    wait.setSingleShot(true);
    wait.onTimeout = [&] { app.quit(); };
    wait.start(2500ms);
    app.run();

    for (int page = 0; page <= 5; ++page) {
        window.setCurrentPage(page);
        window.window().grab().savePng(directory + "/page" + std::to_string(page) + ".png");
        std::ofstream(directory + "/page" + std::to_string(page) + ".txt") << window.window().layoutDump();
    }
    return 0;
}
