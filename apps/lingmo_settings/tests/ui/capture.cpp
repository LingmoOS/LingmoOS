// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <cstdlib>
#include <clocale>
#include <libintl.h>
#include "base/i18n.h"
#include <string>

#include "ui/application.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/picture.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;

// A fixed frame time, so captures show animations at one phase.
static constexpr uint64_t kFrameNs = 1000000000;

// settings-capture DIR [PAGE...]: every page, or the ones named, at the
// reference captures' window size, light and dark, at 100% and 150%, with
// the layout dump beside each picture.
int main(int argc, char** argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s DIR [PAGE...]\n", argv[0]);
        return 1;
    }
    Application application(argc, argv, Application::Platform::Headless);
    settings::setDataDirectory(SETTINGS_SOURCE_DIR);
    if(const char* locale=std::getenv("SETTINGS_CAPTURE_LOCALE_DIR")){setenv("LANGUAGE","zh_CN",1);setlocale(LC_ALL,"en_US.UTF-8");Lingmo::i18n::setDomain("lingmo-settings");bindtextdomain("lingmo-settings",locale);}
    const std::string directory = argv[1];
    std::vector<std::string> ids;
    for (int i = 2; i < argc; ++i)
        ids.push_back(argv[i]);
    if (ids.empty()) {
        for (const settings::PageInfo& page : settings::pages())
            ids.push_back(page.id);
    }
    const Theme light = Theme::current();
    int failures = 0;
    for (const auto& [mode, theme] : {std::pair{"light", light}, std::pair{"dark", Theme::dark()}}) {
        auto captureTheme=theme;
        if(const char* font=std::getenv("SETTINGS_CAPTURE_FONT"))captureTheme.fontFamily=font;
        Theme::setCurrent(captureTheme);
        for (const double scale : {1.0, 1.5}) {
            for (const std::string& id : ids) {
                const settings::PageInfo* page = settings::findPage(id);
                if (!page) {
                    fprintf(stderr, "settings-capture: no page %s\n", id.c_str());
                    ++failures;
                    continue;
                }
                settings::SettingsWindow window;
                window.window().setScale(scale);
                window.window().resize({1000, 820});
                if (!page->name.empty())
                    window.switchToPage(page->name);
                else {
                    window.switchToPage(page->parentId);
                    window.pushPage(page->id);
                }
                window.reveal();
                char name[256];
                snprintf(name, sizeof(name), "%s/%s_%s@%g", directory.c_str(), mode, id.c_str(), scale);
                if (!window.window().grab(kFrameNs).savePng(std::string(name) + ".png")) {
                    fprintf(stderr, "settings-capture: cannot write %s.png\n", name);
                    ++failures;
                }
                if (FILE* file = fopen((std::string(name) + ".txt").c_str(), "w")) {
                    fputs(window.window().layoutDump().c_str(), file);
                    fclose(file);
                }
            }
        }
    }
    Theme::setCurrent(light);
    return failures ? 1 : 0;
}
