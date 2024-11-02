/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QDebug>

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <KF5/BluezQt/bluezqt/request.h>
#include <KF5/BluezQt/bluezqt/mediaplayer.h>
#ifdef BATTERY
#include <KF5/BluezQt/bluezqt/battery.h>
#endif
#include "lingmo-log4qt.h"
#include "common.h"

class LingmoAdapter;

class LingmoDevice: public QObject
{
public:
    explicit LingmoDevice(BluezQt::DevicePtr device, QString adapter);
    ~LingmoDevice();

    int start(void);

    int devConnect(int type = 0);
    int devDisconnect(void);
    QMap<QString, QVariant> getDevAttr(void);
    int setDevAttr(QMap<QString, QVariant>);
    int pairFuncReply(bool);

    void requestPinCode(const BluezQt::Request<QString> &request);
    void requestPasskey(const BluezQt::Request<quint32> &request);
    void requestConfirmation(const QString &passkey, const BluezQt::Request<> &request);
    void requestAuthorization(const BluezQt::Request<> &request);
    void authorizeService(const QString &uuid, const BluezQt::Request<> &request);
    void displayPinCode(const QString &pinCode);

    bool needClean(void);
    bool isAudioType(void);

    int reset(void);

    QString Name(void) { return m_device->name(); }

    bool ispaired(void) { return m_paired; }

    void setSendfileStatus(bool status = true);
protected slots:
    void nameChanged(const QString &name);
    void pairedChanged(bool paired);
    void trustedChanged(bool trusted);
    void blockedChanged(bool blocked);
    void rssiChanged(qint16 rssi);
    void connectedChanged(bool connected);
#ifdef BATTERY
    void batteryChanged(BluezQt::BatteryPtr battery);
    void percentageChanged(int percentage);
#endif
    void typeChanged(BluezQt::Device::Type type);
    void uuidsChanged(const QStringList &uuids);
    void mediaTransportChanged(BluezQt::MediaTransportPtr mediaTransport);  
    //////////////////////////////////////////////
    void pairfinished(BluezQt::PendingCall *call);
    void connectfinished(BluezQt::PendingCall *call);

protected:
    int pair();
    int connectToDevice();

    int updateAudioDevice(void);

    void deal_active_connection(void);

    virtual void timerEvent( QTimerEvent *event);

    void wait_for_finish(BluezQt::PendingCall *call);
private:
    void __send_attr(enum_send_type stype = enum_send_type_delay);
    void __send_add(void);
    void __send_remove(void);
    void __init_uuid();
    void __init_devattr(QMap<QString, QVariant> &);
    void __kill_reconnect_a2dp_timer(void);
protected:
    QString m_adapter;

    int m_need_clean = 0;

    BluezQt::DevicePtr m_device = nullptr;

    bool m_connectProgress = false;
    bool m_pairProgress = false;

    bool m_connect = false;
    bool m_paired = false;

    QString m_showName;

    BluezQt::Request<> m_request;

    QStringList m_uuids;
    bool m_support_ad2p_sink = false;
    bool m_support_a2dp_source = false;
    bool m_support_hfphsp_ag = false;
    bool m_support_hfphsp_hf = false;
    bool m_support_filetransport = false;

    QMap<QString, QVariant> m_attr_send;
    int m_attrsignal_TimerID= 0;

    /**
     * @brief m_connect_type
     *        0 : 主动连接
     *        1 : 回连
     */
    int m_connect_type = 0;

    QString m_lastConnectError;

    bool m_a2dp_connect = false;
    int m_a2dp_reconnect_TimerID = 0;
    int m_a2dp_reconnect_count = 0;

    friend class LingmoAdapter;
};


#endif // DEVICE_H
