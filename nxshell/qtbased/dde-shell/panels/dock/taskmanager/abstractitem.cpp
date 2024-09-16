// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsglobal.h"
#include "itemadaptor.h"
#include "abstractitem.h"

namespace dock {
AbstractItem::AbstractItem(const QString& id, QObject* parent)
    : QObject(parent)
{
    new ItemAdaptor(this);
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.deepin.ds.Dock.TaskManager.Item"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/deepin/ds/Dock/TaskManager/Item/") + id, "org.deepin.ds.Dock.TaskManager.Item", this);
}

}