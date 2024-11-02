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

#ifndef BLUETOOTHDBUS_H
#define BLUETOOTHDBUS_H

#include <glib.h>
#include <gio/gio.h>
#include <QString>
#include <QDebug>
#include <QStringList>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusMessage>
#include <QMap>
#include <QProcess>
#include <QDBusConnectionInterface>
#include <QStandardPaths>
#include "config/config.h"
#include "sessiondbusinterface.h"

#define LOCK_PING_TIME 70
#define LENGTH_OF_TIME_VECTOR 20
#define PINT_TIME_COUNT 20
#define TIME_OUT_COUNT 21
#define LENGTH_OF_ALL_VECTOR 22
#define MAX_TIME_OUT 10

class BluetoothDbus : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothDbus(QObject *parent = nullptr);

    ~BluetoothDbus();

    void setDeviceList(QStringList devicelist);

    void setDeviceType(QStringList typelist);

public:
    //DBus调用函数
    void InitDefaultAdapterDevicesAttr();

    bool getTrayIconShow();

    void setDefaultAdapterPower(bool power);

    void cancelFileTransfer(QString address, int opt);

    bool sendFiles(QString address, QStringList filelist);

    void openBluetoothSettings();

    void devConnect(const QString address);

    void devDisconnect(const QString address);

    void activeConnectionReply(QString dev, bool v);

    void replyFileReceiving(QString address, bool accept);

    void pairFuncReply(QString address, bool accept);

    bool isPowered();

    bool getExistAdapter();

    bool getDevPairStatus(QString address);

    QStringList getOrderList();

    QMap<QString, QMap<QString, QVariant>> getPairedDevices();
    QMap<QString, QMap<QString, QVariant>> getSendableDevices();

signals:
    void devRemoveSignal(QString);

    void devAttrChanged(QString, QMap<QString, QVariant>);

    void showTrayIcon(bool);

    void existAdapter(bool, bool);

    void adapterAddSignal(QString);

    void adapterRemoveSignal(QString);

    void adapterChangedSignal();

    void adapterCanOperat(bool);

    void powerChangedSignal(bool);

    void displayPasskeySignal(QString, QString,QString);

    void requestConfirmationSignal(QString, QString, QString);

    void receiveFilesSignal(QString, QString,QString,QString,quint64);

    void sendTransferFilesMesgSignal(QStringList);

    void sendTransferDeviceMesgSignal(QString);

    void showTrayWidgetUISignal();

    void activeConnectionSignal(QString,QString,QString,int,int);

    void defaultAdapterChanged();

    void deviceAddSignal(QMap<QString, QVariant>);

    void transferredChangedSignal(quint64,QString);

    void statusChangedSignal(QMap<QString, QVariant>);

    void pairAgentCanceledSignal(QString);

    void devBatteryChangedSignal();

    void activeUserChangedSignal(QString);

    void connectionErrorMsg(QString,int);

    void adapterAutoConnChanged(bool);

private:
    int getDevBatteryLevel(QString address);

    void connectDBusSignals();

    void addNewPairedDevice(QString address);

    bool registerClient();

    bool unregisterClient();

    /** Init adapters attr */
    void InitAdaptersAttr();

    void getAdapterAttr(QString address);

    void setAdapterAttrInside(QString address, QMap<QString, QVariant> mapAttr);

    void adapterAttrChanged(QString address, QMap<QString, QVariant> mapAttr);

    /** Init devices attr of default adapter */
    void getDevAttr(QString address);

    void setDeviceAttrInside(QString address, QMap<QString, QVariant> mapAttr);

    void deviceAttrChanged(QString address, QMap<QString, QVariant> mapAttr);

    void leaveLock(QString address, bool on);

    void setLeaveLock(QString address, bool on);

    void getPingTimeList(int i_time);

    SessionDbusInterface *sessDbusInterface = nullptr;
    int leaveLockCount[LENGTH_OF_ALL_VECTOR] = {0};
    int adapterCnt = 0;
    bool leaveLockon = false;
    bool passWrongSignal = false;
    bool timerActive = false;
    bool reping = false;
    QString userName;
    QStringList m_Order;
    QString lockDev;
    QGSettings *gsetting;
    QMap<QString, QVariant> defaultAdapterAttr;
    QMap<QString, QMap<QString, QVariant>> *devicesList;
    QMap<QString, QMap<QString, QVariant>> *pairedList;
    QMap<QString, QMap<QString, QVariant>> *adapterList;

private slots:
    void adapterAddSignalSLot(QMap<QString, QVariant> adapterAttr);

    void adapterChangedSLot(QString address, QMap<QString, QVariant> adapterAttr);

    void adapterAttrChangedSLot(QString address, QMap<QString, QVariant> adapterAttr);

    void updateClientSLot();

    void deviceAddSLot(QMap<QString, QVariant> deviceAttr);

    void deviceAttrChangedSLot(QString address, QMap<QString, QVariant> deviceAttr);

    void devRemoveSignalSLot(QString address, QMap<QString, QVariant> attr);

    void devBatteryChangedSignalSlot(QString address, int battery);

    void adapterRemoveSignalSLot(QString address);

    void receiveFilesSlot(QMap<QString, QVariant> info);

    void devPairSignalSLot(QMap<QString, QVariant> pairAttr);

    bool getDevSupportFileSend(QString address);

    void devErrorSignalSLot(QString);

    void pingTimeSignalSLot(QByteArray);

    void unlockSlot();

    void lockSlot();
};

#endif // BLUETOOTHDBUS_H
