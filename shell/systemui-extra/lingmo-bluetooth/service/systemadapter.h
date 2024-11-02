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

#ifndef SYSTEMADAPTER_H
#define SYSTEMADAPTER_H

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>
#include "lingmo-log4qt.h"

class SystemAdapter : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.lingmo.bluetooth")
public:
    explicit SystemAdapter(QObject *parent = nullptr);
    virtual ~SystemAdapter();

public slots:
    QMap<QString, QVariant> registerClient(QMap<QString, QVariant>);
    bool unregisterClient(QString);
    QStringList getRegisterClient(void);

    QStringList getAllAdapterAddress(void);
    QMap<QString, QVariant> getAdapterAttr(QString, QString);
    QStringList getDefaultAdapterAllDev(void);
    QStringList getDefaultAdapterPairedDev(void);
    QMap<QString, QVariant> getDevAttr(QString);
    bool setDevAttr(QString, QMap<QString, QVariant>);
    bool setDefaultAdapterAttr(QMap<QString, QVariant>);
    bool setLeaveLock(QString uuid, QString dev, bool on);
    int setDefaultAdapter(QString);
    int devConnect(QString);
    int devDisconnect(QString);
    int devRemove(QStringList);
    int activeConnectionReply(QString, bool);
    int pairFuncReply(QString, bool);
    int sendFiles(QString, QString, QStringList);
    int stopFiles(QMap<QString, QVariant>);
    int replyFileReceiving(QMap<QString, QVariant>);

    void writeBuriedPointData(QString, QString, QString, QString);

    QString bluetoothKeyValue(unsigned int,QString);

signals:
    void adapterAddSignal(QMap<QString, QVariant>);
    void adapterAttrChanged(QString, QMap<QString, QVariant>);
    void adapterRemoveSignal(QString);
    void deviceAddSignal(QMap<QString, QVariant>);
    void deviceAttrChanged(QString, QMap<QString, QVariant>);
    void deviceRemoveSignal(QString, QMap<QString, QVariant>);
    void ActiveConnection(QString, QString, QString, int, int);
    void updateClient(void);
    void startPair(QMap<QString, QVariant>);
    void fileStatusChanged(QMap<QString, QVariant>);
    void fileReceiveSignal(QMap<QString, QVariant>);
    void clearBluetoothDev(QStringList);
    void pingTimeSignal(QByteArray);


    //音频控制信号
    void VolumeDown();
    void VolumeUp();
    void Next();
    void PlayPause();
    void Previous();
    void Stop();
    void Play();
    void Pause();

private:
    QString getCallerDebus(void);
};

#endif // SYSTEMADAPTER_H
