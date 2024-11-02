/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "mpris-player-collecter.h"
#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusConnectionInterface>
#include <QDebug>
#include <mutex>

static MprisPlayerCollecter *globalInstance = nullptr;
static std::once_flag flag;
class MprisPlayerCollecterPrivate: public QObject
{
    Q_OBJECT
public:
    explicit MprisPlayerCollecterPrivate(QObject *parent = nullptr);

    void serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);
    void removePlayer(const QString &serviceName);
    void addPlayer(const QString &serviceName);
    void serviceNameFetched(QDBusPendingCallWatcher *watcher);
    QDBusServiceWatcher *m_watcher = nullptr;
    QHash<QString, uint> m_playerServices;
    QHash<QString, PlayerItem *> m_playerItems;
    MprisPlayerCollecter *q = nullptr;

};

MprisPlayerCollecterPrivate::MprisPlayerCollecterPrivate(QObject *parent) : QObject(parent)
{
    m_watcher = new QDBusServiceWatcher(QStringLiteral("org.mpris.MediaPlayer2*"),
                                        QDBusConnection::sessionBus(),
                                        QDBusServiceWatcher::WatchForOwnerChange,
                                        this);
    connect(m_watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &MprisPlayerCollecterPrivate::serviceOwnerChanged);
    QDBusPendingCall async = QDBusConnection::sessionBus().interface()->asyncCall(QStringLiteral("ListNames"));
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, &MprisPlayerCollecterPrivate::serviceNameFetched);
    q = qobject_cast<MprisPlayerCollecter *>(parent);
}

void MprisPlayerCollecterPrivate::serviceOwnerChanged(const QString &serviceName, const QString &oldOwner,
                                                      const QString &newOwner)
{
    if (!serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2."))) {
        return;
    }
//    QString sourceName = serviceName.mid(23);
    if(!oldOwner.isEmpty()) {
        qDebug() << "MPRIS service" << serviceName << "just went offline";
        removePlayer(serviceName);
    }
    if(!newOwner.isEmpty()) {
        qDebug() << "MPRIS service" << serviceName << "just came online";
        addPlayer(serviceName);
    }

}

void MprisPlayerCollecterPrivate::removePlayer(const QString &serviceName)
{
    uint pid = m_playerServices.take(serviceName);
    if(pid > 0) {
        Q_EMIT q->playerRemoved(serviceName, pid);
        auto player = m_playerItems.value(serviceName);
        player->disconnect();
        player->deleteLater();
        m_playerItems.remove(serviceName);
    }
}

void MprisPlayerCollecterPrivate::addPlayer(const QString &serviceName)
{
    QDBusReply<uint> pidReply = QDBusConnection::sessionBus().interface()->servicePid(serviceName);
    if(pidReply.isValid()) {
        m_playerServices.insert(serviceName, pidReply.value());
        Q_EMIT q->playerAdded(serviceName, pidReply.value());
    } else {
        qWarning() << "Can not get pid for service:" << serviceName;
    }
}

void MprisPlayerCollecterPrivate::serviceNameFetched(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QStringList> propsReply = *watcher;
    watcher->deleteLater();

    if (propsReply.isError()) {
        qWarning() << "MprisPlayerCollecter: Could not get list of available D-Bus services";
    } else {
        for (const QString &serviceName: propsReply.value()) {
            if (serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2."))) {
                qDebug() << "Found MPRIS service" << serviceName;
                addPlayer(serviceName);
            }
        }
    }
}

MprisPlayerCollecter::MprisPlayerCollecter(QObject *parent) : QObject(parent), d(new MprisPlayerCollecterPrivate(this))
{
}

PlayerItem *MprisPlayerCollecter::item(uint pid)
{
    //这里我们默认每个pid只对应一个service
    QHashIterator<QString, uint> iter(d->m_playerServices);
    while (iter.hasNext()) {
        iter.next();
        if(iter.value() == pid) {
            return item(iter.key());
        }
    }
    return nullptr;
}

PlayerItem *MprisPlayerCollecter::item(const QString &service)
{
    if(d->m_playerItems.contains(service)) {
        return d->m_playerItems.value(service);
    } else {
        auto item = new PlayerItem(service);
        d->m_playerItems.insert(service, item);
        connect(item, &PlayerItem::dataChanged, this, &MprisPlayerCollecter::dataChanged);
        return item;
    }
}

QStringList MprisPlayerCollecter::playerServices()
{
    return d->m_playerServices.keys();
}

MprisPlayerCollecter *MprisPlayerCollecter::self()
{
    std::call_once(flag, [&] {
        globalInstance = new MprisPlayerCollecter();
    });
    return globalInstance;
}

uint MprisPlayerCollecter::pidOfService(const QString &service)
{
    return d->m_playerServices.value(service);
}


#include "mpris-player-collecter.moc"