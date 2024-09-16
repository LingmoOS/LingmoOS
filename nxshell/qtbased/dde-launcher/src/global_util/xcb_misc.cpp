// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>
#include <QX11Info>

#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

#include "xcb_misc.h"

static XcbMisc * _xcb_misc_instance = nullptr;

XcbMisc::XcbMisc()
{
    xcb_intern_atom_cookie_t * cookie = xcb_ewmh_init_atoms(QX11Info::connection(), &m_ewmh_connection);
    xcb_ewmh_init_atoms_replies(&m_ewmh_connection, cookie, nullptr);
}

XcbMisc::~XcbMisc()
{

}

XcbMisc * XcbMisc::instance()
{
    if (_xcb_misc_instance == nullptr) {
        _xcb_misc_instance = new XcbMisc;
    }

    return _xcb_misc_instance;
}

void XcbMisc::set_window_type(xcb_window_t winId, WindowType winType)
{
    xcb_atom_t atoms[1];

    switch (winType) {
    case WindowType::Desktop:
        atoms[0] = m_ewmh_connection._NET_WM_WINDOW_TYPE_DESKTOP;
        break;
    case WindowType::Dock:
        atoms[0] = m_ewmh_connection._NET_WM_WINDOW_TYPE_DOCK;
        break;
    default:
        break;
    }

    xcb_ewmh_set_wm_window_type(&m_ewmh_connection, winId, 1, atoms);
}

void XcbMisc::set_strut_partial(xcb_window_t winId, Orientation orientation, uint strut, uint start, uint end)
{
    // xcb_ewmh_wm_strut_partial_t strut_partial is very different from
    // xcb_ewmh_wm_strut_partial_t strut_partial {};
    // the latter one ensures all its member to be initialized to 0;
    xcb_ewmh_wm_strut_partial_t strut_partial {};

    switch (orientation) {
    case OrientationLeft:
        strut_partial.left = strut;
        strut_partial.left_start_y = start;
        strut_partial.left_end_y = end;
        break;
    case OrientationRight:
        strut_partial.right = strut;
        strut_partial.right_start_y = start;
        strut_partial.right_end_y = end;
        break;
    case OrientationTop:
        strut_partial.top = strut;
        strut_partial.top_start_x = start;
        strut_partial.top_end_x = end;
        break;
    case OrientationBottom:
        strut_partial.bottom = strut;
        strut_partial.bottom_start_x = start;
        strut_partial.bottom_end_x = end;
        break;
    default:
        break;
    }

    xcb_ewmh_set_wm_strut_partial(&m_ewmh_connection, winId, strut_partial);
}

void XcbMisc::setLauncher(xcb_window_t winId){
    xcb_atom_t atoms_state[2] = {m_ewmh_connection._NET_WM_STATE_SKIP_PAGER, m_ewmh_connection._NET_WM_STATE_SKIP_TASKBAR};
    xcb_atom_t atoms_type[1] = { m_ewmh_connection._NET_WM_WINDOW_TYPE_SPLASH};
    xcb_ewmh_set_wm_window_type(&m_ewmh_connection, winId, 1, atoms_type);
    xcb_ewmh_set_wm_state(&m_ewmh_connection, winId, 2, atoms_state);
}

void XcbMisc::set_deepin_override(xcb_window_t winId)
{
    QString deepinOverride("_DEEPIN_OVERRIDE");

    xcb_connection_t * connection = QX11Info::connection();
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection,
                                                      0,
                                                      deepinOverride.length(),
                                                      deepinOverride.toLatin1());

    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection,
                                                           cookie,
                                                           nullptr);
    if (reply) {
        xcb_atom_t atom = reply->atom;
        free (reply);

        int value = 1;
        xcb_change_property(connection,
                            XCB_PROP_MODE_REPLACE,
                            winId,
                            atom,
                            XCB_ATOM_CARDINAL,
                            32,
                            1,
                            &value);
        xcb_flush(connection);
    }
}
