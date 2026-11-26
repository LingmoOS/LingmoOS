// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#include "photoswindow.h"
#include <ui/application.h>
int main(int argc, char** argv) { const std::string path = argc > 1 ? argv[1] : std::string(); Lingmo::ui::Application app(argc, argv); app.setApplicationId("org.lingmo.Photos"); Photos::PhotosWindow window; if (!path.empty()) window.openFile(path); window.show(); return app.run(); }
