/*
    SPDX-FileCopyrightText: 2014 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NETWORKMANAGERQT_FAKE_NETWORK_ACCESS_POINT_H
#define NETWORKMANAGERQT_FAKE_NETWORK_ACCESS_POINT_H

#include <QObject>

#include "../generictypes.h"

class AccessPoint : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fakenetwork.AccessPoint")
public:
    explicit AccessPoint(QObject *parent = nullptr);
    ~AccessPoint() override;

    Q_PROPERTY(uint Flags READ flags)
    Q_PROPERTY(uint Frequency READ frequency)
    Q_PROPERTY(QString HwAddress READ hwAddress)
    Q_PROPERTY(uint MaxBitrate READ maxBitrate)
    Q_PROPERTY(uint Mode READ mode)
    Q_PROPERTY(uint RsnFlags READ rsnFlags)
    Q_PROPERTY(QByteArray Ssid READ ssid)
    Q_PROPERTY(uchar Strength READ strength)
    Q_PROPERTY(uint WpaFlags READ wpaFlags)

    uint flags() const;
    uint frequency() const;
    QString hwAddress() const;
    uint maxBitrate() const;
    uint mode() const;
    uint rsnFlags() const;
    QByteArray ssid() const;
    uchar strength() const;
    uint wpaFlags() const;

    /* Not part of DBus interface */
    QString accessPointPath() const;
    void setAccessPointPath(const QString &path);
    void setFlags(uint flags);
    void setFrequency(uint frequency);
    void setHwAddress(const QString &hwAddress);
    void setMaxBitrate(uint bitrate);
    void setMode(uint mode);
    void setRsnFlags(uint flags);
    void setSsid(const QByteArray &ssid);
    void setStrength(uchar strength);
    void setWpaFlags(uint flags);

Q_SIGNALS:
    Q_SCRIPTABLE void PropertiesChanged(const QVariantMap &properties);

private:
    QString m_apPath;
    uint m_flags;
    uint m_frequency;
    QString m_hwAddress;
    uint m_maxBitrate;
    uint m_mode;
    uint m_rsnFlags;
    QByteArray m_ssid;
    uchar m_strength;
    uint m_wpaFlags;
};

#endif
