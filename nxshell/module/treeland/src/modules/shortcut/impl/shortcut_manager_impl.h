// Copyright (C) 2023 Dingyuan Zhang <zhangdingyuan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include <wayland-server-core.h>

#include <qwdisplay.h>

#include <QList>
#include <QObject>

struct treeland_shortcut_context_v1;

struct treeland_shortcut_manager_v1 : public QObject
{
    Q_OBJECT
public:
    ~treeland_shortcut_manager_v1();
    wl_event_loop *event_loop{ nullptr };
    wl_global *global{ nullptr };
    QList<wl_resource *> clients;

    QList<treeland_shortcut_context_v1 *> contexts;

    static treeland_shortcut_manager_v1 *create(QW_NAMESPACE::QWDisplay *display);

Q_SIGNALS:
    void newContext(uid_t uid, treeland_shortcut_context_v1 *context);
    void beforeDestroy();
};

struct treeland_shortcut_context_v1 : public QObject
{
    Q_OBJECT
public:
    ~treeland_shortcut_context_v1();
    treeland_shortcut_manager_v1 *manager{ nullptr };
    char *key{ nullptr };
    wl_resource *resource{ nullptr };

    void send_shortcut();
    void send_register_failed();
Q_SIGNALS:
    void beforeDestroy();
};
