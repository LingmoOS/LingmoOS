// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "searchwindow.h"
#include "ui/application.h"
#include "ui/theme.h"
#include <filesystem>
#include <poll.h>
int main(int argc, char** argv) {
    if (argc != 2) return 1;
    std::filesystem::create_directories(argv[1]);
    Lingmo::ui::Application app(argc, argv, Lingmo::ui::Application::Platform::Headless);
    for (bool dark : {false, true}) for (double scale : {1.0, 1.5}) {
        Lingmo::ui::Theme::setCurrent(dark ? Lingmo::ui::Theme::dark() : Lingmo::ui::Theme::light());
        Search::SearchWindow search(false);
        search.setTestSources({{"settings.desktop", "Settings", "System settings", "Personalize your desktop", "", {"display"}},
            {"image-viewer.desktop", "Image Viewer", "Pictures", "View your images", "", {"display"}},
            {"terminal.desktop", "Terminal", "Command line", "", "", {}}}, {});
        search.show(); search.window().setScale(scale);
        const auto suffix = std::string(dark ? "dark" : "light") + (scale == 1 ? "-100" : "-150");
        auto save = [&](const std::string& name) { return search.window().grab().savePng(std::string(argv[1]) + '/' + name + '-' + suffix + ".png"); };
        if (!save("search")) return 2;
        search.setQuery("display");
        auto end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (search.searching() && std::chrono::steady_clock::now() < end) { pollfd fd{app.loop().fd(), POLLIN, 0}; ::poll(&fd, 1, 10); app.loop().dispatch(); }
        if (search.searching() || !save("results")) return 3;
        search.setCategory(Search::Category::Calculator); search.setQuery("(128+64)*2");
        end = std::chrono::steady_clock::now() + std::chrono::seconds(3);
        while (search.searching() && std::chrono::steady_clock::now() < end) { pollfd fd{app.loop().fd(), POLLIN, 0}; ::poll(&fd, 1, 10); app.loop().dispatch(); }
        if (!save("calculation")) return 4;
    }
}
