// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/dbus_menu_popup.h"

#include "shell/statusbar/dbus_menu.h"
#include "ui/icon.h"
#include "ui/menu.h"

namespace Lingmo::Shell {

// What the menus' closures share with the popup: null once it is gone, so a
// menu still on its way out reaches no exporter.
struct DBusMenuPopup::State {
    DBusMenu *dbus = nullptr;
    DBusMenuPopup *owner = nullptr;
    bool prepared = false;
};

DBusMenuPopup::DBusMenuPopup(DBusMenu *menu, int32_t id)
    : m_id(id)
    , m_state(std::make_shared<State>())
    , m_menu(std::make_unique<ui::Menu>())
{
    m_state->dbus = menu;
    m_state->owner = this;
    wire(m_state, *m_menu, m_id);
}

DBusMenuPopup::~DBusMenuPopup()
{
    m_state->dbus = nullptr;
    m_state->owner = nullptr;
    m_menu->onClosed = nullptr;
    m_menu->close();
}

void DBusMenuPopup::detach()
{
    m_state->dbus = nullptr;
}

void DBusMenuPopup::setPrepared()
{
    m_state->prepared = true;
}

std::string DBusMenuPopup::plainLabel(const std::string &label)
{
    std::string text;
    text.reserve(label.size());
    for (size_t i = 0; i < label.size(); ++i) {
        if (label[i] == '_') {
            if (i + 1 < label.size() && label[i + 1] == '_') {
                text += '_';
                ++i;
            }
            continue;
        }
        text += label[i];
    }
    return text;
}

void DBusMenuPopup::wire(const std::shared_ptr<State> &state, ui::Menu &menu, int32_t id)
{
    ui::Menu *target = &menu;
    // The exporter fills the menu on aboutToShow, and the
    // rows show what has arrived so far.
    const bool root = id == (state->owner ? state->owner->m_id : -1);
    menu.onAboutToShow = [state, target, id, root] {
        if (!state->dbus)
            return;
        if (root && state->prepared) {
            state->prepared = false;
            state->dbus->opened(id);
        } else {
            state->dbus->aboutToShow(id);
        }
        fill(state, *target, id);
    };
    menu.onClosed = [state, id, root] {
        if (!state->dbus)
            return;
        state->dbus->closed(id);
        if (root && state->owner && state->owner->onClosed)
            state->owner->onClosed();
    };
}

void DBusMenuPopup::fill(const std::shared_ptr<State> &state, ui::Menu &menu, int32_t id)
{
    menu.clear();
    DBusMenu *dbus = state->dbus;
    const DBusMenuItem *parent = dbus ? dbus->item(id) : nullptr;
    if (!parent)
        return;

    std::vector<const DBusMenuItem *> rows;
    for (int32_t childId : parent->children) {
        const DBusMenuItem *child = dbus->item(childId);
        if (!child || !child->visible)
            continue;
        // A separator is only worth a line between two entries.
        if (child->separator && (rows.empty() || rows.back()->separator))
            continue;
        rows.push_back(child);
    }
    while (!rows.empty() && rows.back()->separator)
        rows.pop_back();

    // Rows are counted as the menu counts them, separators included.
    size_t index = 0;
    for (const DBusMenuItem *row : rows) {
        if (row->separator) {
            menu.addSeparator();
            ++index;
            continue;
        }
        const std::string text = plainLabel(row->label);
        if (row->hasSubmenu || !row->children.empty()) {
            wire(state, *menu.addSubmenu(text), row->id);
            // Its rows ready for when it opens.
            if (!row->childrenLoaded)
                dbus->refresh(row->id);
        } else {
            const int32_t rowId = row->id;
            menu.addItem(text, [state, rowId] {
                if (state->dbus)
                    state->dbus->activate(rowId);
            });
        }
        menu.setItemEnabled(index, row->enabled);
        menu.setItemChecked(index, row->toggle != DBusMenuItem::Toggle::None && row->toggleState == 1);
        // A name the theme has; a picture of the exporter's own is not shown.
        if (!row->iconName.empty() && !ui::themeIconPath(row->iconName, 22).empty())
            menu.setItemIcon(index, row->iconName);
        ++index;
    }
}

} // namespace Lingmo::Shell
