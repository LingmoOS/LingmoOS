// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WM_CONNECTION_H
#define WM_CONNECTION_H

#include "wm_atom.h"
#include "wm_info.h"

#include <xcb/xcb.h>

namespace core {
namespace wm {

class WMConnection
{
public:
    explicit WMConnection(const QByteArray &display = {});
    ~WMConnection();

    inline xcb_atom_t atom(WMAtom::atom_t type) const { return m_atoms.atom(type); }

    xcb_connection_t *xcb_connection() const { return m_conn.get(); }
    WMWId rootWindow() const { return m_root; }

private:
    struct XDisconnector {
        void operator()(xcb_connection_t *conn)
        {
            if (conn && !xcb_connection_has_error(conn))
                xcb_disconnect(conn);
        }
    };
    using XConnection = std::unique_ptr<xcb_connection_t, XDisconnector>;
    XConnection m_conn {};
    const xcb_setup_t *m_setup {};
    xcb_screen_t *m_screen {};
    WMAtom m_atoms;
    int m_screenNumber {};
    WMWId m_root;
};

} // namespace wm
} // namespace core

#endif // WM_CONNECTION_H
