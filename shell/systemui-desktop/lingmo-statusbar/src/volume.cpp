/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "volume.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDBusPendingCall>

static const QString Service = "com.lingmo.Settings";
static const QString ObjectPath = "/Audio";
static const QString Interface = "com.lingmo.Audio";

static VolumeManager *SELF = nullptr;

VolumeManager *VolumeManager::self()
{
    if (!SELF)
        SELF = new VolumeManager;

    return SELF;
}

VolumeManager::VolumeManager(QObject *parent)
    : QObject(parent)
    , m_isValid(false)
    , m_isMute(false)
    , m_volume(0)
{
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(this);
    watcher->setConnection(QDBusConnection::sessionBus());
    watcher->addWatchedService(Service);

    init();

    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &VolumeManager::init);
}

bool VolumeManager::isValid() const
{
    return m_isValid;
}

void VolumeManager::initStatus()
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    m_isValid = iface.isValid() && !iface.lastError().isValid();

    if (m_isValid) {
        int volume = iface.property("volume").toInt();
        bool mute = iface.property("mute").toBool();

        if (m_volume != volume) {
            m_volume = volume;
            emit volumeChanged();
        }

        if (m_isMute != mute) {
            m_isMute = mute;
            emit muteChanged();
        }
    }

    emit validChanged();
}

void VolumeManager::connectDBusSignals()
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    if (iface.isValid()) {
        QDBusConnection::sessionBus().connect(Service, ObjectPath, Interface, "volumeChanged",
                                              this, SLOT(onDBusVolumeChanged(int)));
        QDBusConnection::sessionBus().connect(Service, ObjectPath, Interface, "muteChanged",
                                              this, SLOT(onDBusMuteChanged(bool)));
    }
}

void VolumeManager::onDBusVolumeChanged(int volume)
{
    if (m_volume != volume) {
        m_volume = volume;
        emit volumeChanged();
    }
}

void VolumeManager::onDBusMuteChanged(bool mute)
{
    if (m_isMute != mute) {
        m_isMute = mute;
        emit muteChanged();

        // Need to update the icon.
        emit volumeChanged();
    }
}

int VolumeManager::volume() const
{
    return m_volume;
}

QString VolumeManager::iconName() const
{
    if (m_volume <= 0 || m_isMute)
        return QStringLiteral("audio-volume-muted-symbolic");
    else if (m_volume <= 25)
        return QStringLiteral("audio-volume-low-symbolic");
    else if (m_volume <= 75)
        return QStringLiteral("audio-volume-medium-symbolic");
    else
        return QStringLiteral("audio-volume-high-symbolic");
}

void VolumeManager::toggleMute()
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    if (iface.isValid()) {
        iface.call("toggleMute");
    }
}

void VolumeManager::setMute(bool mute)
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    if (iface.isValid()) {
        iface.call("setMute", QVariant::fromValue(mute));
    }
}

void VolumeManager::setVolume(int value)
{
    QDBusInterface iface(Service, ObjectPath, Interface, QDBusConnection::sessionBus(), this);

    if (iface.isValid()) {
        iface.call("setVolume", QVariant::fromValue(value));
    }
}

void VolumeManager::init()
{
    initStatus();
    connectDBusSignals();
}

bool VolumeManager::isMute() const
{
    return m_isMute;
}
