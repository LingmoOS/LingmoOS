// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifysettings.h"
#include "constants.h"

#include <QStringList>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>

static const QString DDENotifyDBusServer = "org.deepin.dde.Notification1";
static const QString DDENotifyDBusInterface = "org.deepin.dde.Notification1";
static const QString DDENotifyDBusPath = "/org/deepin/dde/Notification1";

NotifySettingsObserver::NotifySettingsObserver(QObject *parent)
    : AbstractNotifySetting(parent)
{
}

void NotifySettingsObserver::setAppSetting(const QString &id, const NotifySettingsObserver::AppConfigurationItem &item, const QVariant &var)
{
    qDebug() << "setAppSetting()" << id << item << var;
    notifyObserver()->call("SetAppInfo", id, static_cast<uint>(item), QVariant::fromValue(QDBusVariant(var)));
}

QVariant NotifySettingsObserver::getAppSetting(const QString &id, const NotifySettingsObserver::AppConfigurationItem &item)
{
    QDBusPendingCall call = notifyObserver()->asyncCall("GetAppInfo", id, static_cast<uint>(item));
    call.waitForFinished();
    QDBusPendingReply<QVariant> reply(call);
    const QVariant &data = reply.value();
    qDebug() << "getAppSetting()" << id << item << data;
    return data;
}

void NotifySettingsObserver::onReceivedAppInfoChanged(const QString &id, uint item, QDBusVariant var)
{
    qDebug() << "onReceivedAppInfoChanged()" << id << item << var.variant();
    Q_EMIT appSettingChanged(id, item, var.variant());
}

QDBusInterface *NotifySettingsObserver::notifyObserver()
{
    if (!m_notifyObserver) {
        m_notifyObserver.reset(new QDBusInterface(DDENotifyDBusServer, DDENotifyDBusPath, DDENotifyDBusInterface));

        if (!m_notifyObserver->isValid()) {
            qWarning() << "notifyObserver() NotifyInterface is invalid, and can't send operator.";
        }

        // TODO it' can be removed when service is registered.
        notifyObserver()->setTimeout(10);

        auto connection = QDBusConnection::sessionBus();
        bool valid = connection.connect(DDENotifyDBusServer, DDENotifyDBusPath, DDENotifyDBusInterface,
                           "AppInfoChanged", this, SLOT(onReceivedAppInfoChanged(const QString &, uint, QDBusVariant)));
        if (!valid) {
            qWarning() << "notifyObserver() NotifyConnection is invalid, and can't receive AppInfoChanged signal."
                       << connection.lastError();
        }
    }
    return m_notifyObserver.get();
}
