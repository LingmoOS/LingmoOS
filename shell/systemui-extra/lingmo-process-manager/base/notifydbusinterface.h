/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef NOTIFYDBUSINTERFACE_H
#define NOTIFYDBUSINTERFACE_H

#include <QObject>
#include <QDBusInterface>

class NotifyDBusInterface : public QObject
{
    Q_OBJECT
public:
    using NotifyCloseCallback = std::function<void(uint,uint)>;
    using ActionInvokedCallback = std::function<void(uint,QString)>;
    using SendNotificationCallback = std::function<void(uint)>;

    explicit NotifyDBusInterface(QObject *parent = nullptr);

    void sendNotificationAsync(
        const QString &summary, const QString &body,
        const QStringList &actions, const QVariantMap &hints, uint replace = 0,
        SendNotificationCallback callback = nullptr);
    void setNotifyCloseCallback(NotifyCloseCallback callback);
    void setActionInvokedCallback(ActionInvokedCallback callback);

private Q_SLOTS:
    void handleNotifyClose(uint notifyId, uint reason);
    void handleActionInvoked(uint notifyId, const QString &actionKey);

private:
    void initNotifyInterface();
    void initConnections();

    QDBusInterface* m_notifyInterface;
    std::vector<NotifyCloseCallback> m_notifyCloseCallbacks;
    std::vector<ActionInvokedCallback> m_actionInvokedCallbacks;
};


#endif // NOTIFYDBUSINTERFACE_H
