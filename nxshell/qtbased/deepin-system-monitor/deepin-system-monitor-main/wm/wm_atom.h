// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WM_ATOM_H
#define WM_ATOM_H

#include <QVector>

#include <xcb/xcb.h>

namespace core {
namespace wm {

class WMAtom
{
public:
    enum atom_t {
        // ICCCM window state
        WM_STATE,
        WM_NAME,
        WM_CLASS,

        _NET_CLIENT_LIST,
        _NET_CLIENT_LIST_STACKING,

        // EWMH
        _NET_WM_DESKTOP,
        _NET_VIRTUAL_ROOTS,

        _NET_WM_NAME,
        _NET_WM_ICON_NAME,
        _NET_WM_ICON,

        _NET_WM_PID,

        _NET_WM_WINDOW_OPACITY,

        _NET_WM_STATE,
        _NET_WM_STATE_ABOVE,
        _NET_WM_STATE_BELOW,
        _NET_WM_STATE_FULLSCREEN,
        _NET_WM_STATE_MAXIMIZED_HORZ,
        _NET_WM_STATE_MAXIMIZED_VERT,
        _NET_WM_STATE_MODAL,
        _NET_WM_STATE_STAYS_ON_TOP,
        _NET_WM_STATE_DEMANDS_ATTENTION,
        _NET_WM_USER_TIME,
        _NET_WM_USER_TIME_WINDOW,
        _NET_WM_FULL_PLACEMENT,
        _NET_WM_STATE_STICKY,
        _NET_WM_STATE_SHADED,
        _NET_WM_STATE_SKIP_TASKBAR,
        _NET_WM_STATE_SKIP_PAGER,
        _NET_WM_STATE_HIDDEN,

        _NET_WM_WINDOW_TYPE,
        _NET_WM_WINDOW_TYPE_DESKTOP,
        _NET_WM_WINDOW_TYPE_DOCK,
        _NET_WM_WINDOW_TYPE_TOOLBAR,
        _NET_WM_WINDOW_TYPE_MENU,
        _NET_WM_WINDOW_TYPE_UTILITY,
        _NET_WM_WINDOW_TYPE_SPLASH,
        _NET_WM_WINDOW_TYPE_DIALOG,
        _NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
        _NET_WM_WINDOW_TYPE_POPUP_MENU,
        _NET_WM_WINDOW_TYPE_TOOLTIP,
        _NET_WM_WINDOW_TYPE_NOTIFICATION,
        _NET_WM_WINDOW_TYPE_COMBO,
        _NET_WM_WINDOW_TYPE_DND,
        _NET_WM_WINDOW_TYPE_NORMAL,

        _NET_FRAME_EXTENTS,

        // property formats
        UTF8_STRING,
        CARDINAL,

        ATOM_MAX
    };

    WMAtom();
    void initialize(xcb_connection_t *connection);
    inline xcb_atom_t atom(WMAtom::atom_t atom) const { return m_atoms[atom]; }

private:
    QVector<xcb_atom_t> m_atoms;
};

} // namespace wm
} // namespace core

#endif // WM_ATOM_H
