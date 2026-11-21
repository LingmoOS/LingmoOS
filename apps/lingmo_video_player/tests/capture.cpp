// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

// Renders the player headless in the reference states, with layout dumps:
//   video-player-capture DIR WIDTH HEIGHT SCALE light|dark CONFIG [VIDEO]
// CONFIG is a video-player.conf to use (its Recent list fills the welcome
// page); VIDEO, a file to open for the playing states.
#include "application.h"
#include "urls.h"
#include "views/mainwindow.h"
#include "views/widgets.h"

#include "base/event_loop.h"
#include "base/timer.h"
#include "ui/application.h"
#include "ui/test_input.h"
#include "ui/theme.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>

using namespace Lingmo;
using namespace std::chrono_literals;

static void runUntil(ui::Application& app, const std::function<bool()>& done, std::chrono::milliseconds timeout)
{
    base::Timer poll;
    int ticks = 0;
    const int limit = int(timeout / 20ms);
    poll.onTimeout = [&] {
        if (done() || ++ticks > limit)
            app.quit();
    };
    poll.start(20ms);
    app.run();
}

static void wait(ui::Application& app, std::chrono::milliseconds time)
{
    runUntil(app, [] { return false; }, time);
}

int main(int argc, char** argv)
{
    if (argc < 7) {
        std::fprintf(stderr, "usage: %s DIR WIDTH HEIGHT SCALE light|dark CONFIG [VIDEO]\n", argv[0]);
        return 1;
    }
    const std::string directory = argv[1];
    const double width = std::atof(argv[2]);
    const double height = std::atof(argv[3]);
    const double scale = std::atof(argv[4]);
    const bool dark = std::string(argv[5]) == "dark";
    const std::string config = argv[6];
    const std::string video = argc > 7 ? argv[7] : std::string();
    std::filesystem::create_directories(directory);

    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    views::registerGlyphs();
    ui::Theme::setCurrent(dark ? ui::Theme::dark() : ui::Theme::light());
    Application application(config, directory + "/covers");
    MainWindow window(application);
    window.window().setScale(scale);
    window.window().resize({width, height});
    window.show();

    const auto capture = [&](const std::string& name) {
        window.window().grab().savePng(directory + "/" + name + ".png");
        std::ofstream(directory + "/" + name + ".txt") << window.window().layoutDump();
    };

    wait(app, 300ms);
    capture("welcome");
    if (video.empty())
        return 0;

    application.openUrls({urls::fromLocalFile(video)});
    runUntil(app, [&] { return window.player().loaded() && window.player().videoActive(); }, 10s);
    window.player().setPaused(true);
    window.player().seek(10, true);
    runUntil(app, [&] { return std::abs(window.player().position() - 10) < 0.05 && !window.player().buffering(); }, 5s);
    wait(app, 500ms);
    capture("paused");

    window.showOsd("Volume 100%");
    wait(app, 300ms);
    capture("osd");
    wait(app, 2000ms);

    for (int tab = 0; tab < 3; ++tab) {
        if (!window.panelOpen())
            window.togglePanel(tab, false);
        else
            window.setPanelTab(tab);
        wait(app, 300ms);
        capture(tab == 0 ? "panel-playlist" : tab == 1 ? "panel-tracks" : "panel-playback");
    }
    window.togglePanel(-1, false);
    wait(app, 300ms);

    // The pointer over the seek bar's middle.
    const double barY = height - 16 - 12 - 10;
    ui::TestInput::movePointer(window.window(), {width / 2, barY});
    wait(app, 1500ms);
    capture("seek-hover");
    ui::TestInput::movePointer(window.window(), {width / 2, height / 2});
    wait(app, 300ms);
    return 0;
}
