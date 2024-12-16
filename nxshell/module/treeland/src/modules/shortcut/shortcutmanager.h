// Copyright (C) 2023 Dingyuan Zhang <zhangdingyuan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#pragma once

#include "helper.h"

#include <wserver.h>

#include <QObject>
#include <QQmlEngine>

class treeland_shortcut_context_v1;
class treeland_shortcut_manager_v1;

class ShortcutV1 : public QObject, public Waylib::Server::WServerInterface
{
    Q_OBJECT

public:
    explicit ShortcutV1(Helper *helper, QObject *parent = nullptr);

protected:
    void create(WServer *server) override;
    void destroy(WServer *server) override;
    wl_global *global() const override;

private Q_SLOTS:
    void onNewContext(uid_t uid, treeland_shortcut_context_v1 *context);

private:
    treeland_shortcut_manager_v1 *m_manager = nullptr;
    Helper *m_helper = nullptr;
};
