/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_ENABLED_CONNECTIONS_H
#define PLASMA_NM_ENABLED_CONNECTIONS_H

#include <QObject>

#include <qqmlregistration.h>

#include <NetworkManagerQt/Manager>

class EnabledConnections : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * Indicates if overall networking is currently enabled or not
     */
    Q_PROPERTY(bool networkingEnabled READ isNetworkingEnabled NOTIFY networkingEnabled)
    /**
     * Indicates if wireless is currently enabled or not
     */
    Q_PROPERTY(bool wirelessEnabled READ isWirelessEnabled NOTIFY wirelessEnabled)
    /**
     * Indicates if the wireless hardware is currently enabled, i.e. the state of the RF kill switch
     */
    Q_PROPERTY(bool wirelessHwEnabled READ isWirelessHwEnabled NOTIFY wirelessHwEnabled)
    /**
     * Indicates if mobile broadband devices are currently enabled or not.
     */
    Q_PROPERTY(bool wwanEnabled READ isWwanEnabled NOTIFY wwanEnabled)
    /**
     * Indicates if the mobile broadband hardware is currently enabled, i.e. the state of the RF kill switch.
     */
    Q_PROPERTY(bool wwanHwEnabled READ isWwanHwEnabled NOTIFY wwanHwEnabled)

public:
    explicit EnabledConnections(QObject *parent = nullptr);
    ~EnabledConnections() override;

    bool isNetworkingEnabled() const;
    bool isWirelessEnabled() const;
    bool isWirelessHwEnabled() const;
    bool isWwanEnabled() const;
    bool isWwanHwEnabled() const;

public Q_SLOTS:
    void onNetworkingEnabled(bool enabled);
    void onWirelessEnabled(bool enabled);
    void onWirelessHwEnabled(bool enabled);
    void onWwanEnabled(bool enabled);
    void onWwanHwEnabled(bool enabled);

Q_SIGNALS:
    void networkingEnabled(bool enabled);
    void wirelessEnabled(bool enabled);
    void wirelessHwEnabled(bool enabled);
    void wwanEnabled(bool enabled);
    void wwanHwEnabled(bool enabled);

private:
    bool m_networkingEnabled;
    bool m_wirelessEnabled;
    bool m_wirelessHwEnabled;
    bool m_wwanEnabled;
    bool m_wwanHwEnabled;
};

#endif // PLASMA_NM_ENABLED_CONNECTIONS_H
