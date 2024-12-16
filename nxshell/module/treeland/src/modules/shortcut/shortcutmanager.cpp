// Copyright (C) 2023 Dingyuan Zhang <zhangdingyuan@uniontech.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "shortcutmanager.h"

#include "impl/shortcut_manager_impl.h"

#include <wayland-server-core.h>
#include <wayland-util.h>

#include <qwdisplay.h>

#include <QAction>

#include <pwd.h>
#include <sys/socket.h>
#include <unistd.h>

ShortcutV1::ShortcutV1(Helper *helper, QObject *parent)
    : QObject(parent)
    , m_helper(helper)
{
}

void ShortcutV1::onNewContext(uid_t uid, treeland_shortcut_context_v1 *context)
{
    QAction *action = new QAction(context);
    action->setShortcut(QString(context->key));

    passwd *pw = getpwuid(uid);
    const QString username(pw->pw_name);

    connect(action, &QAction::triggered, this, [context] {
        context->send_shortcut();
    });

    connect(context, &treeland_shortcut_context_v1::beforeDestroy, this, [this, username, action] {
        m_helper->removeAction(username, action);
    });

    m_helper->addAction(username, action);
}

void ShortcutV1::create(WServer *server)
{
    m_manager = treeland_shortcut_manager_v1::create(server->handle());
    connect(m_manager, &treeland_shortcut_manager_v1::newContext, this, &ShortcutV1::onNewContext);
}

void ShortcutV1::destroy(WServer *server) { }

wl_global *ShortcutV1::global() const
{
    return m_manager->global;
}
