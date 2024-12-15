// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "treelandoutputwatcher.h"
#include "wayland-treeland-output-manager-v1-client-protocol.h"

#include <QGuiApplication>
#include <QScreen>

#include <qpa/qwindowsysteminterface.h>

TreelandOutputWatcher::TreelandOutputWatcher(QObject *parent)
    : QWaylandClientExtensionTemplate<TreelandOutputWatcher>(treeland_output_manager_v1_interface.version)
{
    setParent(parent);
}

TreelandOutputWatcher::~TreelandOutputWatcher()
{
    destroy();
}

void TreelandOutputWatcher::treeland_output_manager_v1_primary_output(const QString &output_name)
{
    if (qApp->primaryScreen()->name() == output_name)
        return;

    for (auto screen : qApp->screens()) {
        if (screen->name() == output_name) {
            QWindowSystemInterface::handlePrimaryScreenChanged(screen->handle());
            return;
        }
    }
}
