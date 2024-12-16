// Copyright (C) 2024 Lu YaNing <luyaning@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "server-protocol.h"
#include <qwsignalconnector.h>
#include <qwdisplay.h>

#include <QObject>

struct treeland_window_management_v1 : public QObject
{
    Q_OBJECT
public:
    ~treeland_window_management_v1();
    wl_global *global{ nullptr };
    wl_list resources;
    uint32_t state = 0; //desktop_state 0: normal, 1: show, 2: preview;

    static treeland_window_management_v1 *create(QW_NAMESPACE::QWDisplay *display);
    void set_desktop(uint32_t state);

Q_SIGNALS:
    void requestShowDesktop(uint32_t state);
    void beforeDestroy();
};
