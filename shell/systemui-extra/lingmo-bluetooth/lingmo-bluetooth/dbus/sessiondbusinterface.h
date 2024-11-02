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

#ifndef SESSIONDBUSINTERFACE_H
#define SESSIONDBUSINTERFACE_H

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>
#include "config/config.h"

class SessionDbusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.bluetooth")
public:
    SessionDbusInterface(QMap<QString, QVariant> *_adapterAttr,
                         QMap<QString, QMap<QString, QVariant> > *_adapterList,
                         QMap<QString, QMap<QString, QVariant> > *_deviceList,
                         QObject *parent = nullptr);

    void emitPairedSignal(QString address, bool paired);

    void emitPoweredSignal(bool powered);

    void setLeaveLockOn(bool leaveLock);

    void setLeaveLockDev(QString lockdev);

    void emitAdapterRemoveSignal(QString address);

    void emitAdapterAddSignal(QString address);

    void emitDefaultAdapterChangedSignal(QString address);

private:
    void ConnectSystemDbusSignal();

    bool InterfaceAlreadyExists();

    QStringList  sessionDbusListNames();

    void controlMprisPlayerDbus(const QString funcName);

    void initMprisMediaDbusConnect();

    bool isLeaveLock = false;
    QString leaveLockDev;
    QMap<QString, QVariant> *adapterAttr = nullptr;
    QMap<QString, QMap<QString, QVariant>> *adapterList = nullptr;
    QMap<QString, QMap<QString, QVariant>> *deviceList = nullptr;

public slots:
    bool registerClient(QString, QString, int, quint64);

    bool unregisterClient(QString);

    bool getBluetoothBlock();

    QString getDefaultAdapterAddress();

    bool getDefaultAdapterPower();

    bool getDefaultAdapterScanStatus();

    bool getDefaultAdapterDiscoverable();

    QString getAdapterNameByAddr(QString);

    QStringList getPairedPhoneAddr();

    QStringList getDefaultAdapterPairedDevAddress();

    QStringList getDefaultAdapterTrustedDevAddress();

    QStringList getDefaultAdapterCacheDevAddress();

    QStringList getAdapterDevAddressList();

    void setDefaultAdapterPower(bool);

    void setDefaultAdapterScanOn(bool);

    void setDefaultAdapter(QString);

    void setDefaultAdapterDiscoverable(bool);

    /** Disabled in newer version */
    void setBluetoothBlock(bool);

    void setDefaultAdapterName(QString);

    void setLeaveLock(QString, bool);

    void clearNonViableDevice(QStringList);

    void devPair(const QString);

    void devConnect(const QString);

    void devDisconnect(const QString);

    void devRemove(const QString);

    void devTrust(const QString,const bool);

    bool getDevPairStatus(const QString);

    bool getDevConnectStatus(const QString);

    QString getDevName(const QString);

    QString getDevType(const QString);

    void clearAllUnPairedDevicelist();

    void setSendTransferDeviceMesg(QString);

    void setSendTransferFileMesg(QStringList files);

    void setclearOldSession();

    void continueSendFiles(QString filename);

    void cancelFileReceiving();

    int getDevBattery(const QString);

    bool getDevSupportFileSend(const QString address);

    int getDevRssi(const QString);

    void sendFiles(QString,QString);

    void exit();

    void sendNotifyMessage(QString);

    void openBluetoothSettings();

    void connectSignal();

    void showTrayWidgetUI();

    bool getTransferDevAndStatus(QString dev);

    void sendContinueRecieveFilesSignal();

    void sendReplyRequestConfirmation(bool);

    void sendReplyFileReceivingSignal(bool v);

    void sendCloseSession();

    void activeConnectionReply(QString dev, bool v);

    bool getLeaveLock();

    QString getLeaveLockDev();

private slots:
    // 操作多媒体影音
    void VolumeDownSlot();

    void VolumeUpSlot();

    void NextSlot();

    void PlayPauseSlot();

    void PreviousSlot();

    void StopSlot();

    void PlaySlot();

    void PauseSlot();

signals:
    void defaultAdapterPowerChanged(bool);

    void defaultAdapterChanged(QString);

    void defaultScanStatusChanged(bool);

    void defaultDiscoverableChanged(bool);

    void adapterAddSignal(QString);

    void adapterRemoveSignal(QString);

    void defaultAdapterNameChanged(QString);

    void deviceScanResult(QString,QString,QString,bool,qint16);

    void devPairSignal(QString,bool);

    void devTypeChangedSignal(QString,QString);

    void devBatteryChangedSignal(QString, QString);

    void devNameChangedSignal(QString,QString);

    void devConnectStatusSignal(QString,bool);

    void devRemoveSignal(QString);

    void devLaunchConnecting(QString);

    void devOperateErrorSignal(QString,int,QString);

    void devMacAddressChangedSignal(QString,QString);

    void devRssiChangedSignal(QString,qint16);

    void sendFile(QString,QString);

    void continueSendFilesSignal(QString);

    void displayPasskey(QString,QString);

    void requestConfirmation(QString,QString);

    void replyRequestConfirmation(bool);

    void transferredChanged(quint64,QString);

    void statusChanged(QString,QString,QString);

    void fileReceivingSignal(QString,QString,QString,QString,quint64);

    void replyFileReceivingSignal(bool);

    void continueRecieveFilesSignal();

    void closeSessionSignal();

    void pairAgentCanceled();

    void obexAgentCanceled();

    void propertyChanged(quint64);

    void sendTransferDeviceMesg(QString);

    void sendTransferFilesMesg(QStringList);

    void clearOldSession();

    void cancelFileReceivingSignal();

    void showTrayWidgetUISignal();

    void initTransferPath(QString, bool);

    void powerProgress(bool);

    void clearBluetoothDev(QStringList);

    void ActiveConnection(QString, QString, QString, int, int);

    void setLeaveLockSignal(QString, bool);
};

#endif // SESSIONDBUSINTERFACE_H
