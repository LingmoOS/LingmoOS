// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wm_atom.h"

#include <QByteArrayList>

#include "xcb/xcb.h"

namespace core {
namespace wm {

WMAtom::WMAtom()
    : m_atoms(ATOM_MAX)
{
}

void WMAtom::initialize(xcb_connection_t *connection)
{
    const char *atomNames[] = {
        "WM_STATE\0",
        "WM_NAME\0",
        "WM_CLASS\0",
        "_NET_CLIENT_LIST\0",
        "_NET_CLIENT_LIST_STACKING\0",
        "_NET_WM_DESKTOP\0",
        "_NET_VIRTUAL_ROOTS\0",
        "_NET_WM_NAME\0",
        "_NET_WM_ICON_NAME\0",
        "_NET_WM_ICON\0",
        "_NET_WM_PID\0",
        "_NET_WM_WINDOW_OPACITY\0",
        "_NET_WM_STATE\0",
        "_NET_WM_STATE_ABOVE\0",
        "_NET_WM_STATE_BELOW\0",
        "_NET_WM_STATE_FULLSCREEN\0",
        "_NET_WM_STATE_MAXIMIZED_HORZ\0",
        "_NET_WM_STATE_MAXIMIZED_VERT\0",
        "_NET_WM_STATE_MODAL\0",
        "_NET_WM_STATE_STAYS_ON_TOP\0",
        "_NET_WM_STATE_DEMANDS_ATTENTION\0",
        "_NET_WM_USER_TIME\0",
        "_NET_WM_USER_TIME_WINDOW\0",
        "_NET_WM_FULL_PLACEMENT\0",
        "_NET_WM_STATE_STICKY\0",
        "_NET_WM_STATE_SHADED\0",
        "_NET_WM_STATE_SKIP_TASKBAR\0",
        "_NET_WM_STATE_SKIP_PAGER\0",
        "_NET_WM_STATE_HIDDEN\0",
        "_NET_WM_WINDOW_TYPE\0",
        "_NET_WM_WINDOW_TYPE_DESKTOP\0",
        "_NET_WM_WINDOW_TYPE_DOCK\0",
        "_NET_WM_WINDOW_TYPE_TOOLBAR\0",
        "_NET_WM_WINDOW_TYPE_MENU\0",
        "_NET_WM_WINDOW_TYPE_UTILITY\0",
        "_NET_WM_WINDOW_TYPE_SPLASH\0",
        "_NET_WM_WINDOW_TYPE_DIALOG\0",
        "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU\0",
        "_NET_WM_WINDOW_TYPE_POPUP_MENU\0",
        "_NET_WM_WINDOW_TYPE_TOOLTIP\0",
        "_NET_WM_WINDOW_TYPE_NOTIFICATION\0",
        "_NET_WM_WINDOW_TYPE_COMBO\0",
        "_NET_WM_WINDOW_TYPE_DND\0",
        "_NET_WM_WINDOW_TYPE_NORMAL\0",
        "_NET_FRAME_EXTENTS\0",
        "UTF8_STRING\0",
        "CARDINAL\0",
        "\0\0"};

    xcb_intern_atom_cookie_t cookies[ATOM_MAX];
    int i;
    for (i = 0; i < ATOM_MAX && atomNames[i]; i++)
        cookies[i] = xcb_intern_atom(connection, false, uint16_t(strlen(atomNames[i])), atomNames[i]);

    for (i = 0; i < ATOM_MAX; i++) {
        auto *reply = xcb_intern_atom_reply(connection, cookies[i], nullptr);
        m_atoms[i] = reply->atom;
        free(reply);
    }
}

} // namespace wm
} // namespace core
