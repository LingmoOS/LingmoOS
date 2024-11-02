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

#ifndef SESSIONDBUSREGISTER_H
#define SESSIONDBUSREGISTER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include "lingmo-log4qt.h"
#include "CSingleton.h"


class SystemAdapter;

class SysDbusMng : public QObject
{
    Q_OBJECT
protected:
    explicit SysDbusMng(QObject *parent = nullptr);
    ~SysDbusMng();

public:
    int start(void);

    int stop(void);

    SystemAdapter *get_sys_adapter(){ return m_sys_adapter; }

public:
    bool registerClient(QMap<QString, QVariant>, QString);
    bool unregisterClient(QString);

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
protected:
    void sendMultimediaControlButtonSignal(unsigned int);

private:
    SystemAdapter * m_sys_adapter = nullptr;

    QMap<unsigned int, long long> m_pressTimeMap;     //记录键值按下的时间
    QMap<unsigned int, long long> m_releaseTimeMap;   //记录键值释放的时间

    friend class SingleTon<SysDbusMng>;
};

typedef SingleTon<SysDbusMng>  SYSDBUSMNG;

#endif // SESSIONDBUSREGISTER_H
