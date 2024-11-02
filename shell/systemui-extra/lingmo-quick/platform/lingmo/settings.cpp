/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "settings.h"
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QDBusPendingReply>
#include <mutex>
#include <QGuiApplication>
static LingmoUIQuick::Settings *g_settings = nullptr;
static std::once_flag onceFlag;

const QString USD_SERVICE = QStringLiteral("org.lingmo.SettingsDaemon");
const QString USD_GLOBAL_SIGNAL_PATH = QStringLiteral("/GlobalSignal");
const QString USD_GLOBAL_SIGNAL_INTERFACE = QStringLiteral("org.lingmo.SettingsDaemon.GlobalSignal");
const QString LINGMO_LITE_CHANGED = QStringLiteral("LINGMOLiteChanged");
static const QString STATUS_MANAGER_SERVICE = QStringLiteral("com.lingmo.statusmanager.interface");
static const QString STATUS_MANAGER_PATH = QStringLiteral("/");
static const QString STATUS_MANAGER_INTERFACE = QStringLiteral("com.lingmo.statusmanager.interface");
static const QString MODE_CHANGE_SIGNAL = QStringLiteral("mode_change_signal");
namespace LingmoUIQuick {
class SettingsPrivate : public QObject
{
    Q_OBJECT
public:
    explicit SettingsPrivate(QObject *parent = nullptr);
    void updateLiteLevel(const QVariantMap &map);
    void updateTabletMode(bool tabletMode);
    void refresh();
    QString m_liteAnimation = QStringLiteral("normal");
    QString m_liteFunction = QStringLiteral("normal");
    bool m_isTabletMode = false;
    QDBusInterface *m_usdGlobalSignalIface = nullptr;
    QDBusInterface *m_statusManagerIface = nullptr;
    Settings *q = nullptr;
};

SettingsPrivate::SettingsPrivate(QObject *parent) : QObject(parent)
{
    q = qobject_cast<Settings *>(parent);
    refresh();
}

void SettingsPrivate::updateLiteLevel(const QVariantMap &map)
{
    if(map.find(QStringLiteral("lingmo-lite-animation")) != map.constEnd()) {
        m_liteAnimation = map.value(QStringLiteral("lingmo-lite-animation")).toString();
        Q_EMIT q->liteAnimationChanged();
    }
    if(map.find(QStringLiteral("lingmo-lite-function")) != map.constEnd()) {
        m_liteFunction = map.value(QStringLiteral("lingmo-lite-function")).toString();
        Q_EMIT q->liteFunctionChanged();
    }
}

void SettingsPrivate::updateTabletMode(bool tabletMode)
{
    if(m_isTabletMode != tabletMode) {
        m_isTabletMode = tabletMode;
        Q_EMIT q->tabletModeChanged();
    }
}

void SettingsPrivate::refresh()
{
    //lingmo lite
    if(!m_usdGlobalSignalIface) {
        m_usdGlobalSignalIface = new QDBusInterface(USD_SERVICE, USD_GLOBAL_SIGNAL_PATH, USD_GLOBAL_SIGNAL_INTERFACE,
                                                    QDBusConnection::sessionBus(), this);
    }
    if(!m_usdGlobalSignalIface->isValid()) {
        qWarning() << "Init USD interface error:" << m_usdGlobalSignalIface->lastError();
    } else {
        QDBusPendingCall animationCall = m_usdGlobalSignalIface->asyncCall(QStringLiteral("getLINGMOLiteLevel"));
        connect(new QDBusPendingCallWatcher(animationCall, this), &QDBusPendingCallWatcher::finished, [&](QDBusPendingCallWatcher *call = nullptr){
            QDBusPendingReply<QVariantMap> reply = *call;
            if (reply.isError()) {
                qWarning() << "USD interface error" << reply.error();
            } else {
                updateLiteLevel(reply.value());
            }
            call->deleteLater();
        });
        if(!QDBusConnection::sessionBus().connect(USD_SERVICE, USD_GLOBAL_SIGNAL_PATH, USD_GLOBAL_SIGNAL_INTERFACE,
                                                  LINGMO_LITE_CHANGED,
                                                  this, SLOT(updateLiteLevel(const QVariantMap &map)))) {
            qWarning() << "USD interface error, connect " << LINGMO_LITE_CHANGED << "failed!";
        }
    }
    //tablet mode
    if(!m_statusManagerIface) {
        m_statusManagerIface = new QDBusInterface(STATUS_MANAGER_SERVICE, STATUS_MANAGER_PATH, STATUS_MANAGER_INTERFACE,
                                                  QDBusConnection::sessionBus(), this);
    }
    if(!m_statusManagerIface->isValid()) {
        qWarning() << "Init lingmo status manager interface error:" << m_statusManagerIface->lastError();
    } else {
        QDBusPendingCall tabletCall = m_statusManagerIface->asyncCall(QStringLiteral("get_current_tabletmode"));
        connect(new QDBusPendingCallWatcher(tabletCall, this), &QDBusPendingCallWatcher::finished, [&](QDBusPendingCallWatcher *call = nullptr){
            QDBusPendingReply<bool> reply = *call;
            if (reply.isError()) {
                qWarning() << "Lingmo status manager interface error" << reply.error();
            } else {
                updateTabletMode(reply.value());
            }
            call->deleteLater();
        });
        if(!QDBusConnection::sessionBus().connect(STATUS_MANAGER_SERVICE, STATUS_MANAGER_PATH, STATUS_MANAGER_INTERFACE,
                                                  MODE_CHANGE_SIGNAL,
                                                  this, SLOT(updateTabletMode(bool)))) {
            qWarning() << "Lingmo status manager interface error, connect " << MODE_CHANGE_SIGNAL << "failed!";
        }
    }
}

Settings *Settings::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_settings = new Settings();
    });
    return g_settings;
}

LingmoUIQuick::Settings::Settings(QObject *parent) : QObject(parent), d(new SettingsPrivate(this))
{
    refresh();
}

QString LingmoUIQuick::Settings::liteAnimation()
{
    return d->m_liteAnimation;
}

QString LingmoUIQuick::Settings::liteFunction()
{
    return d->m_liteFunction;
}

bool Settings::tabletMode()
{
    return d->m_isTabletMode;
}

void Settings::refresh()
{
    d->refresh();
}

QString Settings::platformName()
{
    return QGuiApplication::platformName();
}
}
#include "settings.moc"