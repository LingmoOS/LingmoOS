// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "base/i18n.h"
#include "clockwindow.h"
#include "ui/application.h"
#include <cstdio>
#include <string>
int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--version") {
            std::puts("Lingmo Clock " LINGMO_CLOCK_VERSION);
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            std::puts("Usage: lingmo-clock [--version]");
            return 0;
        }
    }
    Lingmo::ui::Application app(argc, argv);
    app.setApplicationId("lingmo-clock");
    Lingmo::I18n::setDomain("lingmo-clock");
    Clock::ClockWindow window;
    window.show();
    return app.run();
}
