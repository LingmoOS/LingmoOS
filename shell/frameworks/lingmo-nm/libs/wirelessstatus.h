// SPDX-FileCopyrightText: 2023 Yari Polla <skilvingr@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef WIRELESSSTATUS_H
#define WIRELESSSTATUS_H

#include <QObject>

#include <qqmlregistration.h>

class WirelessStatus : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * Returns the SSID of the currently active hotspot, if any, otherwise an empty string
     */
    Q_PROPERTY(QString hotspotSSID READ hotspotSSID NOTIFY hotspotSSIDChanged)
    /**
     * Returns the SSID of the currently active wireless connection, if any, otherwise an empty string
     */
    Q_PROPERTY(QString wifiSSID READ wifiSSID NOTIFY wifiSSIDChanged)
public:
    explicit WirelessStatus(QObject *parent = nullptr);
    ~WirelessStatus() override;

    QString wifiSSID() const;
    QString hotspotSSID() const;

private Q_SLOTS:
    void activeConnectionsChanged();
    void stateChanged();

Q_SIGNALS:
    void hotspotSSIDChanged(const QString &ssid);
    void wifiSSIDChanged(const QString &ssid);

private:
    QString m_wifiSSID;
    QString m_hotspotSSID;
};

#endif // WIRELESSSTATUS_H
