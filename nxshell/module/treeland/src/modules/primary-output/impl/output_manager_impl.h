// Copyright (C) 2023 rewine <luhongxu@lingmo.org>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "server-protocol.h"

#include <qwsignalconnector.h>
#include <qwdisplay.h>

#include <QObject>

#define TREELAND_OUTPUT_MANAGER_V1_VERSION 1

struct treeland_output_manager_v1 : public QObject
{
    Q_OBJECT
public:
    ~treeland_output_manager_v1();
    wl_global *global;
    wl_list resources;
    const char *primary_output_name{ nullptr };

    static treeland_output_manager_v1 *create(QW_NAMESPACE::QWDisplay *display);
    void set_primary_output(const char *name);

Q_SIGNALS:
    void requestSetPrimaryOutput(const char *name);
    void beforeDestroy();
};
