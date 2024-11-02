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

#ifndef DEVICEBASE_H
#define DEVICEBASE_H


#include <QList>
#include <QDebug>
#include <QObject>
#include <QVector>
#include <QString>
#include <lingmo-log4qt.h>

#include "config.h"

class devicebase;
class bluetoothadapter;
class bluetoothdevice;

//devicebase
class devicebase : public QObject
{
    Q_OBJECT
public:
    devicebase();
    virtual ~devicebase(){}

    virtual void resetDeviceName(QString)    = 0 ;
    virtual QString getDevName()      = 0;
    virtual QString getDevAddress()   = 0;

private:
    QString m_dev_name;
    QString m_dev_address;
};
//devicebase end


//bluetoothadapter
class bluetoothadapter : public devicebase
{
    Q_OBJECT
public:
    bluetoothadapter(QString dev_name        ,
                     const QString dev_address     ,
                     bool    dev_block       ,
                     bool    dev_power       ,
                     bool    dev_pairing     ,
                     bool    dev_pairable    ,
                     bool    dev_connecting  ,
                     bool    dev_discovering ,
                     bool    dev_discoverable,
                     bool    dev_activeConnection,
                     bool    dev_defaultAdapterMark,
                     bool    dev_trayShow);

    bluetoothadapter(QMap<QString ,QVariant> value);

    ~bluetoothadapter(){
        //KyDebug() << "======" << m_dev_name << m_dev_address;
    }

    void resetDeviceName(QString)  Q_DECL_OVERRIDE ;
    QString getDevName()    Q_DECL_OVERRIDE ;
    QString getDevAddress() Q_DECL_OVERRIDE ;

    void setAdapterPower(bool);
    bool getAdapterPower();

    void setAdapterPairing(bool);
    bool getAdapterPairing();

    void setAdapterConnecting(bool);
    bool getAdapterConnecting();

    void setAdapterPairable(bool);
    bool getAdapterPairable();

    void setAdapterDiscovering(bool);
    bool getAdapterDiscovering();

    void setAdapterDiscoverable(bool);
    bool getAdapterDiscoverable();

    void setAdapterAutoConn(bool);
    bool getAdapterAutoConn();

    void setAdapterDefaultMark(bool);
    bool getAdapterDefaultMark();

    void setAdapterTrayShow(bool);
    bool getAdapterTrayShow();

    QMap <QString,bluetoothdevice *> m_bt_dev_list;
//    QList<bluetoothdevice *> m_bluetooth_device_list;
    QMap <QString,bluetoothdevice *> m_bt_dev_paired_list;
//    QList<bluetoothdevice *> m_bluetooth_device_paired_list;

signals:
    void adapterNameChanged(const QString &name);
    void adapterPoweredChanged(bool powered);
    void adapterPairingChanged(bool pairing);
    void adapterPairableChanged(bool pairable);
    void adapterConnectingChanged(bool connecting);

    void adapterTrayIconChanged(bool status);
    void adapterDiscoverableChanged(bool discoverable);
    void adapterDiscoveringChanged(bool discovering);
    void adapterAutoConnStatuChanged(bool status);
    void adapterDeviceAdded(bluetoothdevice * &device);
    void adapterDeviceRemoved(bluetoothdevice * &device);
    void defaultAdapterChanged(const QString &address);

private:
    QString m_dev_name                  ;
    QString m_dev_address               ;

    bool    m_dev_block                 ;
    bool    m_dev_power                 ;
    bool    m_dev_pairing               ;
    bool    m_dev_pairable              ;
    bool    m_dev_connecting            ;
    bool    m_dev_discovering           ;
    bool    m_dev_discoverable          ;
    bool    m_dev_activeConnection      ;
    bool    m_dev_defaultAdapterMark    ;
    bool    m_dev_trayShow              ;

    void    bluetoothAdapterDataAnalysis(QMap<QString ,QVariant> value);
};
//bluetoothadapter end

//bluetoothdevice
class bluetoothdevice : public devicebase
{
    Q_OBJECT

public:
    enum DEVICE_TYPE{
        /** The device is a phone. */
        phone = 0,
        /** The device is a modem. */
        modem,
        /** The device is a computer. */
        computer,
        /** The device is a network. */
        network,
        /** The device is a headset. */
        headset,
        /** The device is a headphones. */
        headphones,
        /** The device is an uncategorized audio video device. */
        audiovideo,
        /** The device is a keyboard. */
        keyboard,
        /** The device is a mouse. */
        mouse,
        /** The device is a joypad. */
        joypad,
        /** The device is a graphics tablet (input device). */
        tablet,
        /** The deivce is an uncategorized peripheral device. */
        peripheral,
        /** The device is a camera. */
        camera,
        /** The device is a printer. */
        printer,
        /** The device is an uncategorized imaging device. */
        imaging,
        /** The device is a wearable device. */
        wearable,
        /** The device is a toy. */
        toy,
        /** The device is a health device. */
        health,
        /** The device is not of any of the known types. */
        uncategorized
    };
    Q_ENUM(DEVICE_TYPE)


    enum Error {
        /** Indicates there is no error. */
        NoError = 0,
        /** Indicates that the device is not ready. */
        NotReady = 1,
        /** Indicates that the action have failed. */
        Failed = 2,
        /** Indicates that the action was rejected. */
        Rejected = 3,
        /** Indicates that the action was canceled. */
        Canceled = 4,
        /** Indicates that invalid arguments were passed. */
        InvalidArguments = 5,
        /** Indicates that an agent or pairing record already exists. */
        AlreadyExists = 6,
        /** Indicates that an agent, service or pairing operation does not exists. */
        DoesNotExist = 7,
        /** Indicates that the action is already in progress. */
        InProgress = 8,
        /** Indicates that the action is not in progress. */
        NotInProgress = 9,
        /** Indicates that the device is already connected. */
        AlreadyConnected = 10,
        /** Indicates that the connection to the device have failed. */
        ConnectFailed = 11,
        /** Indicates that the device is not connected. */
        NotConnected = 12,
        /** Indicates that the action is not supported. */
        NotSupported = 13,
        /** Indicates that the caller is not authorized to do the action. */
        NotAuthorized = 14,
        /** Indicates that the authentication was canceled. */
        AuthenticationCanceled = 15,
        /** Indicates that the authentication have failed. */
        AuthenticationFailed = 16,
        /** Indicates that the authentication was rejected. */
        AuthenticationRejected = 17,
        /** Indicates that the authentication timed out. */
        AuthenticationTimeout = 18,
        /** Indicates that the connection attempt have failed. */
        ConnectionAttemptFailed = 19,
        /** Indicates that the data provided generates a data packet which is too long. */
        InvalidLength = 20,
        /** Indicates that the action is not permitted (e.g. maximum reached or socket locked). */
        NotPermitted = 21,
        /** Indicates an error with D-Bus. */
        DBusError = 98,
        /** Indicates an internal error. */
        InternalError = 99,
        /** Indicates an unknown error. */
        UnknownError = 100
    };
    Q_ENUM(Error)

    bluetoothdevice(QString         dev_address             ,
                    QString         dev_name                ,
                    QString         dev_showName            ,
                    DEVICE_TYPE     dev_type                ,
                    bool            dev_paired              ,
                    bool            dev_trusted             ,
                    bool            dev_blocked             ,
                    bool            dev_connected           ,
                    bool            dev_pairing             ,
                    bool            dev_connecting          ,
                    int             dev_battery             ,
                    int             dev_connectFailedId     ,
                    QString         dev_connectFailedDisc   ,
                    qint16          dev_rssi                ,
                    bool            dev_sendFileMark        ,
                    QString         adapter_address);
    bluetoothdevice(QMap<QString ,QVariant> value);

    ~bluetoothdevice(){
        //KyDebug() << m_dev_name << m_dev_address;
    }

    void resetDeviceName(QString)  Q_DECL_OVERRIDE ;
    QString getDevName()    Q_DECL_OVERRIDE ;
    QString getDevAddress() Q_DECL_OVERRIDE ;

    void setDevShowName(QString);
    QString getDevShowName();
    QString getDevInterfaceShowName();

    void setDevType(DEVICE_TYPE);
    DEVICE_TYPE getDevType();

    bool isPaired();
    void devPairedChanged(bool);

    bool isConnected();
    void devConnectedChanged(bool);

    void setDevPairing(bool);
    bool getDevPairing();//暂时不使用

    void setDevConnecting(bool);
    bool getDevConnecting();

    void devMacAddressChanged(QString);

    void setDevTrust(bool);
    bool getDevTrust();

    void setDevConnFailedInfo(int,QString);

    void setDevSendFileMark(bool);
    bool getDevSendFileMark();

    qint16 getDevRssi();
    void setDevRssi(qint16);

    int getErrorId();

    void setRemoving(bool);
    bool getRemoving();

signals:

    void nameChanged(QString);
    void showNameChanged(QString);
    void rssiChanged(qint16);
    void typeChanged(DEVICE_TYPE);
    void pairedChanged(bool);
    void connectedChanged(bool);
    void pairingChanged(bool);
    void connectingChanged(bool);
    void trustChanged(bool);
    void errorInfoRefresh(int,QString);
    void devConnectingSignal();

private:

    QString         m_dev_address               ;
    QString         m_dev_name                  ;
    QString         m_dev_showName              ;
    DEVICE_TYPE     m_dev_type                  ;
    bool            m_dev_paired     = false    ;
    bool            m_dev_trusted    = false    ;
    bool            m_dev_blocked    = false    ;
    bool            m_dev_connected  = false    ;
    bool            m_dev_pairing    = false    ;//暂时不使用
    bool            m_dev_connecting = false    ;
    int             m_dev_battery               ;
    int             m_dev_connectFailedId       ;
    QString         m_dev_connectFailedDisc     ;
    qint16          m_dev_rssi                  ;
    bool            m_dev_sendFileMark          ;
    QString         m_adapter_address           ;//暂时不使用
    bool            m_dev_removing   = false    ;//正在移除该设备

    void bluetoothDeviceDataAnalysis(QMap<QString ,QVariant> value);

};
//bluetoothdevice end

#endif // DEVICEBASE_H
