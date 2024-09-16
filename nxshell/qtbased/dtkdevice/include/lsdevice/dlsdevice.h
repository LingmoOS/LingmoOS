// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DLSDEVICE_H
#define DLSDEVICE_H

#include "dtkdevice_global.h"

#include <QMap>
#include <QObject>

DDEVICE_BEGIN_NAMESPACE

class DlsDevicePrivate;
class LIBDTKDEVICESHARED_EXPORT DlsDevice : public QObject
{
public:
    enum DevClass {
        DtkUnkown = 0,
        DtkBridge,   DtkController, DtkHub,
        DtkBios,     DtkBus,        DtkAddress, DtkVolume,
        DtkVirtual,
        DtkGeneric,
        DtkProductSystem,
        DtkMainboard,
        DtkCpu,
        DtkMemory,
        DtkStorage,  DtkDisk,
        DtkDvd,      DtkTape,
        DtkNetwork,  Dtkwlan,   DtkCommunication,
        DtkDisplayGPU,         DtkMonitor,
        DtkInput,
        DtkMouse,
        DtkKeyboard,
        DtkFingerprint,
        DtkCamera,
        DtkSoundAudio,
        DtkTouchscreen,
        DtkTouchpad,
        DtkBluetooth,
        DtkPrinter,
        DtkPower,    DtkBattery,
        DtkSensor,
        DtkLed,
        DtkSwitchbuttion,
        DtkScanner,
        DtkChipcard,

        DtkOther,
        DtkRevers1,
        DtkRevers2,

        DtkMax = 100
    };

    struct DDeviceInfo {
        QMap<QString, QString>  deviceInfoLstMap;
        QString vendorName;
        QString productName;
        QString modalias;
        QString vendorID;
        QString productID;
        QString sysFsPath;
        QString baseClassName;
        QString subClassName;
        QString description;
        QStringList deviceBaseAttrisLst;
        QStringList deviceOtherAttrisLst;
        DevClass devClass;
    };

    explicit DlsDevice(QObject *parent = nullptr);
    ~DlsDevice();

    QList< DDeviceInfo >  devicesInfosAll();
    QStringList  deviceAttris(DevClass etype);
    QList< DDeviceInfo > deviceInfo(DevClass etype);
    QList< DDeviceInfo >  deviceInfo(DevClass etype, const int idex);
    int devicesCount() ;
    int devicesCount(DevClass devclass);
    double updateSystemCpuUsage();
    QMap<QString, int> CpuStat();
    double getCpuUsage();
    QList< DDeviceInfo >  deviceCPU();
    QList< DDeviceInfo >  deviceStorage();
    QList< DDeviceInfo >  deviceGPU();
    QList< DDeviceInfo >  deviceMemory();
    QList< DDeviceInfo >  deviceMonitor(); //todo
    QList< DDeviceInfo >  deviceAudio();
    QList< DDeviceInfo >  deviceNetwork();
    QList< DDeviceInfo >  deviceCamera();
    QList< DDeviceInfo >  deviceKeyboard();
    QList< DDeviceInfo >  deviceMouse();
    QList< DDeviceInfo >  deviceComputer();

private:
    QScopedPointer<DlsDevicePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DlsDevice)
};

DDEVICE_END_NAMESPACE

#endif


