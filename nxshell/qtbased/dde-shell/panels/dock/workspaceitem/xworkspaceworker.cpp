// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "xworkspaceworker.h"
#include "workspacemodel.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(workspaceItem, "dde.shell.dock.workspaceItem")

namespace dock {

XWorkspaceWorker::XWorkspaceWorker(WorkspaceModel *model)
    : QObject(model)
    , m_interKwinProp(new QDBusInterface("org.kde.KWin", "/VirtualDesktopManager", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus(), this))
    , m_model(model)
    , m_currentIndex(0)
{
    qDBusRegisterMetaType<DBusDesktopDataStruct>();
    qDBusRegisterMetaType<DBusDesktopDataVector>();
    updateData();

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qDebug() << "DBus session bus connection failed.";
        return;
    }

    bus.connect("org.kde.KWin", "/VirtualDesktopManager", "org.kde.KWin.VirtualDesktopManager", "currentChanged", this, SLOT(updateData()));
    bus.connect("org.kde.KWin", "/VirtualDesktopManager", "org.kde.KWin.VirtualDesktopManager", "desktopsChanged", this, SLOT(updateData()));
    bus.connect("org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1", "org.deepin.dde.Appearance1", "Changed", this, SLOT(appearanceChanged(const QString,const QString)));
    connect(m_model, &WorkspaceModel::currentIndexChanged, this, &XWorkspaceWorker::setIndex);
}

void XWorkspaceWorker::setIndex(int index)
{
    if (m_desktops.count() > index && m_currentIndex != index)
        m_interKwinProp->call("Set", "org.kde.KWin.VirtualDesktopManager", "current", QVariant::fromValue(QDBusVariant(m_desktops[index].id)));
}

void XWorkspaceWorker::appearanceChanged(const QString &changedStr, const QString &value)
{
    if ("allwallpaperuris" == changedStr)
        updateData();
}

void XWorkspaceWorker::updateData()
{
    QDBusMessage reply = m_interKwinProp->call("Get", "org.kde.KWin.VirtualDesktopManager", "desktops");
    if (reply.type() == QDBusMessage::ReplyMessage) {
        QVariant result = reply.arguments().at(0);
        m_desktops = qdbus_cast<DBusDesktopDataVector>(result.value<QDBusVariant>().variant().value<QDBusArgument>());
    } else {
        qCWarning(workspaceItem) << "Error calling desktops";
    }

    reply = m_interKwinProp->call("Get", "org.kde.KWin.VirtualDesktopManager", "current");

    if (reply.type() == QDBusMessage::ReplyMessage) {
        QVariant result = reply.arguments().at(0);
        m_currentId = result.value<QDBusVariant>().variant().toString();
    } else {
        qCWarning(workspaceItem) << "Error calling current";
    }



    QList<WorkSpaceData*> items;
    for (int i = 0; i < m_desktops.count(); ++i) {
        QString image;
        if (m_desktops[i].id == m_currentId) {
            m_currentIndex = i;
            QDBusReply<QString> res = QDBusInterface("org.deepin.dde.Appearance1", "/org/deepin/dde/Appearance1", "org.deepin.dde.Appearance1").call("GetCurrentWorkspaceBackground");
            if (res.error().message().isEmpty()) {
                image = res.value();
            } else {
                qCWarning(workspaceItem) << "GetWorkspaceBackgroundForMonitor failed:" << res.error().message();
                return;
            }
        }
        // kwin workspace name (m_desktops[i].name) no gui to set, so hardcode workspace name for x11

        auto item = new WorkSpaceData(tr("Workspace").append(" " + QString::number(i + 1)), image);
        items.append(item);
    }
    m_model->setItems(items);
    m_model->setCurrentIndex(m_currentIndex);
}
}
