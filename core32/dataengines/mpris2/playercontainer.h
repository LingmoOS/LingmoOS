/*
    SPDX-FileCopyrightText: 2008-2012 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <Plasma/DataContainer>
#include <QFlags>

class OrgFreedesktopDBusPropertiesInterface;
class OrgMprisMediaPlayer2Interface;
class OrgMprisMediaPlayer2PlayerInterface;
class QDBusPendingCallWatcher;

class PlayerContainer : public Plasma::DataContainer
{
    Q_OBJECT

public:
    explicit PlayerContainer(const QString &busAddress, QObject *parent = nullptr);

    QString dbusAddress() const
    {
        return m_dbusAddress;
    }
    OrgFreedesktopDBusPropertiesInterface *propertiesInterface() const
    {
        return m_propsIface;
    }
    OrgMprisMediaPlayer2Interface *rootInterface() const
    {
        return m_rootIface;
    }
    OrgMprisMediaPlayer2PlayerInterface *playerInterface() const
    {
        return m_playerIface;
    }

    enum Cap {
        NoCaps = 0,
        CanQuit = 1 << 0,
        CanRaise = 1 << 1,
        CanSetFullscreen = 1 << 2,
        CanControl = 1 << 3,
        CanPlay = 1 << 4,
        CanPause = 1 << 5,
        CanSeek = 1 << 6,
        CanGoNext = 1 << 7,
        CanGoPrevious = 1 << 8,
        // CanStop is not directly provided by the spec,
        // but we infer it from PlaybackStatus and CanControl
        CanStop = 1 << 9,
    };
    Q_DECLARE_FLAGS(Caps, Cap)
    Caps capabilities() const
    {
        return m_caps;
    }

    enum UpdateType {
        FetchAll,
        UpdatedSignal,
    };

    void refresh();
    void updatePosition();

Q_SIGNALS:
    void initialFetchFinished(PlayerContainer *self);
    void initialFetchFailed(PlayerContainer *self);
    void capsChanged(Caps newCaps);

private Q_SLOTS:
    void getPropsFinished(QDBusPendingCallWatcher *watcher);
    void getPositionFinished(QDBusPendingCallWatcher *watcher);
    void propertiesChanged(const QString &interface, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);
    void seeked(qlonglong position);

private:
    void copyProperty(const QString &propName, const QVariant &value, QVariant::Type expType, UpdateType updType);
    void updateFromMap(const QVariantMap &map, UpdateType updType);
    void recalculatePosition();

    Caps m_caps;
    int m_fetchesPending;
    QString m_dbusAddress;
    OrgFreedesktopDBusPropertiesInterface *m_propsIface;
    OrgMprisMediaPlayer2Interface *m_rootIface;
    OrgMprisMediaPlayer2PlayerInterface *m_playerIface;
    double m_currentRate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlayerContainer::Caps)
