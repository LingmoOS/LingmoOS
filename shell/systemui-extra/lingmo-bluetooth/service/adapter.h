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

#ifndef ADAPTER_H
#define ADAPTER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QDebug>
#include <QSharedPointer>
#include <QTimer>

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>

#include "lingmo-log4qt.h"
#include "CSingleton.h"
#include "common.h"


class LingmoDevice;

typedef QSharedPointer<LingmoDevice> LingmoDevicePtr;

class LingmoAdapter : public QObject
{
    Q_OBJECT
public:
    explicit LingmoAdapter(BluezQt::AdapterPtr adapter);
    ~LingmoAdapter();

    int start(void);

    int stop(void);

    int bluetooth_block(void);

    void set_need_reconnect(bool v){ m_need_reconnect = v; }

    int reconnect_dev(void);
public:
    QStringList getAllDev(void);

    QStringList getPairedDev(void);

    QString addr(void) { return m_adapter->address(); }

    bool setDiscoverable(bool);
    bool setPower(bool);
    bool setActiveConnection(bool);
    bool setName(QString);
    bool trayShow(bool);

    QMap<QString, QVariant> getAdapterAttr(QString);

    LingmoDevicePtr getDevPtr(QString);
    bool devIsAudioType(QString);

    /*
     **** type 连接模式
     *    0 ： 主动连接
     *    1 ：回连
     */
    int devConnect(QString addr, int type = 0);
    int devDisconnect(QString);
    int devRemove(QStringList);


    int updateDiscovering(void);

    int devDisconnectAll(void);

    QString connected_audio_device(void){ return m_connected_audio_device; }
    void connected_audio_device(QString v);

    int activeConnectionReply(QString, bool);

    void clearPinCode(void);
protected slots:
    void nameChanged(const QString &name);
    void poweredChanged(bool powered);
    void discoverableChanged(bool discoverable);
    void discoveringChanged(bool discovering);
    void deviceAdded(BluezQt::DevicePtr device);
    void deviceRemoved(BluezQt::DevicePtr device);
    void uuidsChanged(const QStringList &uuids);

protected:
    int add(BluezQt::DevicePtr adapter);

    int remove(BluezQt::DevicePtr adapter);

    bool deal_active_connection(QString, int);

    bool __setDiscoverable();
    bool __setPower();
    bool __setActiveConnection();
    bool __setDiscovering();

    void __sendAttr(enum_send_type stype = enum_send_type_delay);
    void __sendAdd();

    void __reconnectFunc();

    void __clear_timers();

    int __devRemove(QString);

    virtual void timerEvent( QTimerEvent *event);

    void wait_for_finish(BluezQt::PendingCall *call);

    void __init_uuid();

    void __init_devattr(QMap<QString, QVariant> &);
protected:
    //正在使用的蓝牙适配器标志，仅有一个
    bool m_is_start = false;

    BluezQt::AdapterPtr m_adapter = nullptr;

    QMap<QString, LingmoDevicePtr> m_devices;

    bool m_connectProgress = false;

    bool m_need_reconnect = true;
    bool m_reconnectProgress = false;

    bool m_powerProgress = false;
    bool m_set_power = false;

    bool m_set_discoverable = false;

    bool m_set_Discovering = false;

    bool m_set_activeConnection = false;

    QString m_connected_audio_device;

    QMap<QString, QVariant> m_attr_send;

    int m_set_TimerID = 0;
    int m_autoclean_TimerID = 0;
    int m_attrsignal_TimerID= 0;
    int m_activeConnection_TimerID = 0;
    int m_reconnect_TimerId = 0;

    QString m_active_dev;
    QSet<QString> m_black_activeconn_dev;

    QStringList m_uuids;
    bool m_support_ad2p_sink = false;
    bool m_support_a2dp_source = false;
    bool m_support_hfphsp_ag = false;
    bool m_support_hfphsp_hf = false;
    bool m_support_filetransport = false;

    bool m_cleanProgress = false;

    friend class LingmoDevice;
};

typedef QSharedPointer<LingmoAdapter> LingmoAdapterPtr;

class AdapterMng : public QObject
{
    Q_OBJECT
public:
    AdapterMng();
    ~AdapterMng();

    int add(BluezQt::AdapterPtr adapter);

    int remove(BluezQt::AdapterPtr adapter);

    QStringList getAllAdapterAddress(void);

    QStringList getDefaultAdapterAllDev(void);

    QStringList getDefaultAdapterPairedDev(void);

    int setDefaultAdapter(QString);

    QMap<QString, QVariant> getAdapterAttr(QString, QString);

    LingmoAdapterPtr getDefaultAdapter(void){ return m_default_adapter; }

    void bluetooth_block(bool);

    bool isDefaultAdapter(QString);
protected:
    int update_adapter(void);

protected:
    LingmoAdapterPtr m_default_adapter = nullptr;

    QMap<QString, LingmoAdapterPtr> m_adapters;

    bool m_bluetooth_block = false;

    friend class SingleTon<AdapterMng>;
};
typedef SingleTon<AdapterMng>  ADAPTERMNG;
#endif // ADAPTER_H
