// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wm_connection.h"
#include "ddlog.h"
#include <QDebug>
using namespace DDLog;

namespace core {
namespace wm {

WMConnection::WMConnection(const QByteArray &display)
{
    m_conn = XConnection(xcb_connect(display.isEmpty() ? nullptr : display.constData(), &m_screenNumber));
    auto *conn = m_conn.get();
    if (conn && xcb_connection_has_error(conn)) {
        qCDebug(app) << "Unable to connect to X server";
        return;
    }
    m_atoms.initialize(conn);

    m_setup = xcb_get_setup(conn);
    auto iter = xcb_setup_roots_iterator(m_setup);
    auto screenCount = xcb_setup_roots_length(m_setup);
    if (screenCount < m_screenNumber) {
        qCDebug(app) << QString("Unable to access to screen %1, max is %2").arg(m_screenNumber).arg(screenCount - 1);
        return;
    }

    // get the screen we want from a list of screens in linked list structure
    for (int i = 0; i < m_screenNumber; i++)
        xcb_screen_next(&iter);
    m_screen = iter.data;

    m_root = m_screen->root;
}

WMConnection::~WMConnection()
{
}

}   // namespace wm
}   // namespace core
