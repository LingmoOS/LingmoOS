// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DBUSNOTIFY_H
#define DBUSNOTIFY_H
#include "commondef.h"
#include <QtDBus/QtDBus>

class DBusNotify : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit DBusNotify(const QString &service, const QString &path, const QString &interface, const QDBusConnection &connection, QObject *parent = nullptr);
    inline int Notify(const QList<QVariant> &argumentList)
    {
        QDBusMessage reply = callWithArgumentList(QDBus::Block, QStringLiteral("Notify"), argumentList);

        int notifyid = -1;
        if (QDBusMessage::ReplyMessage == reply.type()) {
            qCDebug(ServiceLogger) << reply.type() << reply.errorName() << reply.errorMessage();
            QDBusReply<quint32> id = reply;
            if (id.isValid()) {
                notifyid = id.value();
            }
        } else {
            qCDebug(ServiceLogger) << reply.type() << reply.errorName() << reply.errorMessage();
        }
        return notifyid;
    }

    /**
     * @brief closeNotification 根据通知ID关闭桌面顶部通知
     * @param notifyID          通知ID
     */
    void closeNotification(quint32 notifyID);
Q_SIGNALS: // SIGNALS
    Q_SCRIPTABLE void NotificationClosed(quint32 id, quint32 reason);
    Q_SCRIPTABLE void ActionInvoked(quint32 id, const QString &reason);
};

#endif // DBUSNOTIFY_H
