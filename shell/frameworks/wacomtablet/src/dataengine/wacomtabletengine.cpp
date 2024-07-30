/*
 * Copyright (C) 2015 Weng Xuetian <wengxt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "wacomtabletengine.h"
#include "multidbuspendingcallwatcher.h"
#include "wacomtabletservice.h"

using namespace Wacom;

WacomTabletEngine::WacomTabletEngine(QObject *parent, const QVariantList &args)
    : DataEngine(parent)
    , m_source(QLatin1String("wacomtablet"))
{
    // init dbus service watcher
    QDBusServiceWatcher *dbusServiceWatcher = new QDBusServiceWatcher(this);
    dbusServiceWatcher->setWatchedServices(QStringList(QLatin1String("org.kde.Wacom")));
    dbusServiceWatcher->setWatchMode(QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
    dbusServiceWatcher->setConnection(QDBusConnection::sessionBus());

    connect(dbusServiceWatcher, SIGNAL(serviceRegistered(QString)), this, SLOT(onDBusConnected()));
    connect(dbusServiceWatcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(onDBusDisconnected()));

    onDBusConnected();
}

WacomTabletEngine::~WacomTabletEngine()
{
}

void WacomTabletEngine::onDBusConnected()
{
    DBusTabletInterface::resetInterface();

    if (!DBusTabletInterface::instance().isValid()) {
        onDBusDisconnected();
        return;
    }

    setData(m_source, QLatin1String("serviceAvailable"), true);

    connect(&DBusTabletInterface::instance(), SIGNAL(tabletAdded(QString)), this, SLOT(onTabletAdded(QString)));
    connect(&DBusTabletInterface::instance(), SIGNAL(tabletRemoved(QString)), this, SLOT(onTabletRemoved(QString)));
    connect(&DBusTabletInterface::instance(), SIGNAL(profileChanged(QString, QString)), this, SLOT(setProfile(QString, QString)));

    // get list of connected tablets
    QDBusReply<QStringList> connectedTablets = DBusTabletInterface::instance().getTabletList();

    foreach (const QString &tabletId, connectedTablets.value()) {
        onTabletAdded(tabletId);
    }
}

void WacomTabletEngine::onDBusDisconnected()
{
    setData(m_source, QLatin1String("serviceAvailable"), false);
    const auto keys = m_tablets.keys();
    for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
        onTabletRemoved(*iter);
    }
    m_tablets.clear();
}

void WacomTabletEngine::onTabletAdded(const QString &tabletId)
{
    if (m_tablets.contains(tabletId)) {
        return;
    }

    QDBusReply<bool> isTouchSensor = DBusTabletInterface::instance().isTouchSensor(tabletId);
    if (isTouchSensor.isValid() && isTouchSensor.value()) {
        return;
    }

    QList<QDBusPendingCall> callList;

    callList << DBusTabletInterface::instance().getInformation(tabletId, TabletInfo::TabletName.key()) << DBusTabletInterface::instance().listProfiles(tabletId)
             << DBusTabletInterface::instance().getProfile(tabletId)
             << DBusTabletInterface::instance().getProperty(tabletId, DeviceType::Stylus.key(), Property::Mode.key())
             << DBusTabletInterface::instance().getDeviceName(tabletId, DeviceType::Touch.key())
             << DBusTabletInterface::instance().getProperty(tabletId, DeviceType::Touch.key(), Property::Touch.key());

    MultiDBusPendingCallWatcher *watcher = new MultiDBusPendingCallWatcher(callList, this);
    connect(watcher, &MultiDBusPendingCallWatcher::finished, [this, watcher, tabletId](const QList<QDBusPendingCallWatcher *> &watchers) {
        watcher->deleteLater();
        Q_FOREACH (auto watcher, watchers) {
            if (watcher->isError()) {
                return;
            }
        }

        QDBusPendingReply<QString> deviceName = *watchers[0];
        QDBusPendingReply<QStringList> profileListReply = *watchers[1];
        QDBusPendingReply<QString> profileName = *watchers[2];
        QDBusPendingReply<QString> stylusMode = *watchers[3];
        QDBusPendingReply<QString> hasTouch = *watchers[4];
        QDBusPendingReply<QString> touchMode = *watchers[5];

        const QString sourceName = QString(QLatin1String("Tablet%1")).arg(tabletId);
        auto &tabletData = m_tablets[tabletId];
        tabletData.name = deviceName.value();
        const auto profileList = profileListReply.value();
        tabletData.profiles = profileList;
        int item = profileList.indexOf(profileName.value());
        tabletData.currentProfile = item;
        tabletData.hasTouch = (hasTouch.isValid() && !hasTouch.value().isEmpty());
        tabletData.touch = tabletData.hasTouch ? QString(touchMode).contains(QLatin1String("on")) : false;
        tabletData.stylusMode = (QString(stylusMode).contains(QLatin1String("absolute")) || QString(stylusMode).contains(QLatin1String("Absolute")));

        setData(sourceName, QLatin1String("currentProfile"), tabletData.currentProfile);
        setData(sourceName, QLatin1String("hasTouch"), tabletData.hasTouch);
        setData(sourceName, QLatin1String("touch"), tabletData.touch);
        setData(sourceName, QLatin1String("profiles"), tabletData.profiles);
        setData(sourceName, QLatin1String("stylusMode"), tabletData.stylusMode);
        setData(sourceName, QLatin1String("name"), tabletData.name);
        setData(sourceName, QLatin1String("id"), tabletId);
    });
}

void WacomTabletEngine::onTabletRemoved(const QString &tabletId)
{
    const QString sourceName = QString(QLatin1String("Tablet%1")).arg(tabletId);
    m_tablets.remove(tabletId);
    removeSource(sourceName);
}

void WacomTabletEngine::setProfile(const QString &tabletId, const QString &profile)
{
    if (!m_tablets.contains(tabletId)) {
        return;
    }
    int item = m_tablets[tabletId].currentProfile = m_tablets[tabletId].profiles.indexOf(profile);
    const QString sourceName = QString(QLatin1String("Tablet%1")).arg(tabletId);
    setData(sourceName, QLatin1String("currentProfile"), item);
}

Plasma::Service *WacomTabletEngine::serviceForSource(const QString &source)
{
    if (source == m_source) {
        return new WacomTabletService(source, this);
    }

    return Plasma::DataEngine::serviceForSource(source);
}

K_PLUGIN_CLASS_WITH_JSON(WacomTabletEngine, "plasma-dataengine-wacomtablet.json")

#include "wacomtabletengine.moc"

#include "moc_wacomtabletengine.cpp"
