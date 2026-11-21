// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "libraryscanner.h"
#include "photostore.h"
#include <memory>
#include <string>
#include <functional>
namespace Lingmo::ui { class Window; class View; class AboutDialog; }
namespace Photos {
class GlobalMenu;
class PhotosWindow {
public:
    explicit PhotosWindow(std::string database = {});
    ~PhotosWindow();
    void show();
    void showAbout();
    void setMenuAction(std::function<void(int)> action) { m_menuAction = std::move(action); }
    void setViewerActive(bool active);
    Lingmo::ui::Window& window();
private:
    void refresh();
    PhotoStore m_store;
    LibraryScanner m_scanner;
    std::unique_ptr<GlobalMenu> m_menu;
    std::unique_ptr<Lingmo::ui::AboutDialog> m_about;
    std::unique_ptr<Lingmo::ui::Window> m_window;
    Lingmo::ui::View* m_root = nullptr;
    std::function<void(int)> m_menuAction;
};
} // namespace Photos
