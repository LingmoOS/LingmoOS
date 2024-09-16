// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef XCB_MISC_H
#define XCB_MISC_H

#include <xcb/xcb_ewmh.h>

class XcbMisc
{

public:
    enum Orientation {
        OrientationLeft,
        OrientationRight,
        OrientationTop,
        OrientationBottom
    };

    enum WindowType {
        Dock,
        Desktop
    };

    virtual ~XcbMisc();

    static XcbMisc * instance();

    void set_window_type(xcb_window_t winId, WindowType winType);
    void set_strut_partial(xcb_window_t winId, Orientation orientation, uint strut, uint start, uint end);
    void setLauncher(xcb_window_t winId);
    static void set_deepin_override(xcb_window_t winId);

private:
    XcbMisc();

    xcb_ewmh_connection_t m_ewmh_connection;
};

#endif // XCB_MISC_H
