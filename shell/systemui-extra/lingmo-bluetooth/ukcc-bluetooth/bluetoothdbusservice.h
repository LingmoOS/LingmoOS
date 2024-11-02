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

#ifndef BLUETOOTHDBUSSERVICE_H
#define BLUETOOTHDBUSSERVICE_H

#include <QList>
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QDBusReply>
#include <QDBusMessage>
#include <lingmo-log4qt.h>
#include <QStandardPaths>
#include <QDBusInterface>
#include <QDBusConnection>

#include "config.h"
#include "devicebase.h"
#include "ukccbluetoothconfig.h"
/**
 * @brief
 *
 */
class BlueToothDBusService : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief
     *
     * @param parent
     */
    BlueToothDBusService(QObject *parent = nullptr);
    /**
     * @brief
     *
     */
    ~BlueToothDBusService();
    int initBluetoothServer();

    static QDBusInterface interface; /**< TODO: systemd dbus */

    static bool m_taskbar_show_mark ; /**< TODO: default bt adapter */
    static QStringList m_bluetooth_adapter_address_list ; /**< TODO: default bt adapter */
    static QStringList m_bluetooth_adapter_name_list ; /**< TODO: default bt adapter */

    static QStringList m_bluetooth_Paired_Device_address_list ; /**< TODO: default bt adapter */
    static QStringList m_bluetooth_All_Device_address_list ; /**< TODO: default bt adapter */
    static bluetoothadapter * m_default_bluetooth_adapter ; /**< TODO: default bt adapter */
    QList<bluetoothadapter *> m_bluetooth_adapter_list ; /**< TODO: bt adapter list*/

    static int  checkAddrList(QStringList &);
    static QMap<QString, QVariant> registerClient();
    static QMap<QString, QVariant> registerClient(QMap<QString, QVariant>);
    static bool unregisterClient();

    //Function operation
    static int devConnect(QString address);
    static bool devRename(QString address,QString reanme_str);
    static bool setDevTrusted(QString address,bool isTrusted);

    static int devDisconnect(QString address);
    static int devRemove(QString address);
    static int devRemove(QStringList addressList);
    static int sendFiles(QString address);

    //get adapter data
    static QMap<QString, QVariant> defaultAdapterDataAttr;
    static QStringList getAllAdapterAddress();
    QMap<QString ,QVariant> getAdapterAttr(QString,QString);
    static bool setDefaultAdapterAttr(QMap<QString, QVariant>);

    int getAdapterAllData(QString);
    void bluetoothAdapterDataAnalysis(QMap<QString ,QVariant> value,
                                                   QString &dev_name,
                                                   QString &dev_address,
                                                   bool    &dev_block ,
                                                   bool    &dev_power,
                                                   bool    &dev_pairing     ,
                                                   bool    &dev_pairable    ,
                                                   bool    &dev_connecting  ,
                                                   bool    &dev_discovering ,
                                                   bool    &dev_discoverable,
                                                   bool    &dev_activeConnection,
                                                   bool    &dev_defaultAdapterMark,
                                                   bool    &dev_trayShow);
    //set default adapter
    static void setDefaultAdapterName(QString);
    static void setDefaultAdapterSwitchStatus(bool);
    static int  setDefaultAdapter(QString);
    static void setTrayIconShowStatus(bool);
    static void setDefaultAdapterDiscoverableStatus(bool);
    static void setAutoConnectAudioDevStatus(bool);

    //get device data
    static QMap<QString, QVariant> deviceDataAttr;

    static QStringList getDefaultAdapterPairedDev();
    static QStringList getDefaultAdapterAllDev();

    static QMap<QString,QVariant> getDevAttr(QString);
    static bool setDevAttr(QString,QMap<QString,QVariant>);

    void bluetoothDeviceDataAnalysis(QMap<QString ,QVariant> value,
                                     QString         &dev_address             ,
                                     QString         &dev_name                ,
                                     QString         &dev_showName            ,
                                     bluetoothdevice::DEVICE_TYPE     &dev_type                ,
                                     bool            &dev_paired              ,
                                     bool            &dev_trusted             ,
                                     bool            &dev_blocked             ,
                                     bool            &dev_connected           ,
                                     bool            &dev_pairing             ,
                                     bool            &dev_connecting          ,
                                     int             &dev_battery             ,
                                     int             &dev_connectFailedId     ,
                                     QString         &dev_connectFailedDisc   ,
                                     qint16          &dev_rssi                ,
                                     bool            &dev_sendFileMark        ,
                                     QString         &adapter_addr            );

    bluetoothdevice * createOneBleutoothDeviceForAddress(QString address);
    //set device data
    /**
     * @brief
     *
     * @param dev
     * @return bool
     */
    static bool getTransferInfo(QString dev);
    /**
     * @brief setDefaultAdapterScanOn
     *
     * @param bool
     */
    static void setDefaultAdapterScanOn(bool);

    //report data

signals:

    void adapterAddSignal(QString);
    void adapterRemoveSignal(int);
    void defaultAdapterChangedSignal(int);

    void adapterNameChanged(QString);
    void adapterNameChangedOfIndex(int ,QString);
    void adapterPoweredChanged(bool);
    void adapterTrayIconChanged(bool);
    void adapterDiscoverableChanged(bool);
    void adapterActiveConnectionChanged(bool);
    void adapterDiscoveringChanged(bool);

    void deviceAddSignal(QString);
    void deviceRemoveSignal(QString);
    void devicePairedSuccess(QString);

    void btServiceRestart();
    void btServiceRestartComplete(bool);

private Q_SLOTS:

    void reportAdapterAddSignal(QMap<QString ,QVariant>);
    void reportAdapterAttrChanged(QString,QMap<QString ,QVariant>);
    void reportAdapterRemoveSignal(QString address);
    void reportDeviceAddSignal(QMap<QString ,QVariant>);
    void reportDeviceAttrChanged(QString,QMap<QString ,QVariant>);
    int reportDeviceRemoveSignal(QString devAddr , QMap<QString ,QVariant> );
    void reportClearBluetoothDev(QStringList);
    void reportUpdateClient();

    void devLoadingTimeoutSlot();

private:
    /**
     * @brief
     *
     */
//    int getDevListIndex(QString address);

    void bindServiceReportData();
    void bindDefaultAdapterReportData();
    void serviceChangedDefaultAdapter(int);
    void getDefaultAdapterDevices();
    QStringList m_remainder_loaded_bluetooth_device_address_list ;

    QTimer * m_loading_dev_timer = nullptr;
};

#endif // BLUETOOTHDBUSSERVICE_H
