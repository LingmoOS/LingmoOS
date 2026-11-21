// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"

namespace Lingmo::ui {
class Menu;
}

namespace Lingmo::Shell {

class DBusMenu;

// One menu of a
// com.canonical.dbusmenu exporter shown as a FishUI menu. Rows are built
// from the latest layout whenever the menu or one of its submenus opens;
// the exporter hears AboutToShow and the "opened" and "closed" events, and
// a chosen row is activated. Hidden entries have no row, and separators
// only stand between two entries.
class DBusMenuPopup
{
public:
    DBusMenuPopup(DBusMenu *menu, int32_t id);
    ~DBusMenuPopup();

    DBusMenuPopup(const DBusMenuPopup &) = delete;
    DBusMenuPopup &operator=(const DBusMenuPopup &) = delete;

    ui::Menu &menu() { return *m_menu; }

    // The exporter is gone: nothing reaches it from here on.
    void detach();
    // The opener already asked AboutToShow for the menu and waited for its
    // entries: its next opening only sends "opened".
    void setPrepared();

    // The menu closed, by a choice, Escape or a press elsewhere.
    std::function<void()> onClosed;

    // A label as the menu shows it: the exporter's "_" mnemonic markers
    // dropped, "__" a literal underscore.
    static std::string plainLabel(const std::string &label);

private:
    struct State;

    static void fill(const std::shared_ptr<State> &state, ui::Menu &menu, int32_t id);
    static void wire(const std::shared_ptr<State> &state, ui::Menu &menu, int32_t id);

    int32_t m_id;
    std::shared_ptr<State> m_state;
    std::unique_ptr<ui::Menu> m_menu;
};

} // namespace Lingmo::Shell

