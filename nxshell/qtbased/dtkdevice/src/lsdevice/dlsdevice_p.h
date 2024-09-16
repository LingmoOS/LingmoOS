// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef DLSDEVICE_P_H
#define DLSDEVICE_P_H

#include "dlsdevice.h"

#include "hw.h"

#include <QMap>

DDEVICE_BEGIN_NAMESPACE

class DlsDevicePrivate : public QObject
{
    // Q_OBJECT
    Q_DECLARE_PUBLIC(DlsDevice)

public:
    explicit DlsDevicePrivate(DlsDevice *parent);
    virtual ~DlsDevicePrivate()
    {
    }

    void addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo >   &ll);
    DlsDevice::DevClass convertClass(hw::hwClass cc);
    hw::hwClass convertClass(DlsDevice::DevClass cc);

    QList< DlsDevice::DDeviceInfo >  deviceCPU();
    QList< DlsDevice::DDeviceInfo >  deviceStorage();
    QList< DlsDevice::DDeviceInfo >  deviceGPU();
    QList< DlsDevice::DDeviceInfo >  deviceMemory();
    QList< DlsDevice::DDeviceInfo >  deviceMonitor();
    QList< DlsDevice::DDeviceInfo >  deviceAudio();
    QList< DlsDevice::DDeviceInfo >  deviceNetwork();
    QList< DlsDevice::DDeviceInfo >  deviceCamera();
    QList< DlsDevice::DDeviceInfo >  deviceKeyboard();
    QList< DlsDevice::DDeviceInfo >  deviceMouse();
    QList< DlsDevice::DDeviceInfo >  deviceComputer();

    double updateSystemCpuUsage();
    QMap<QString, int> CpuStat();
    double getCpuUsage();

public:
    QList< DlsDevice::DDeviceInfo >      m_listDeviceInfo;
    hwNode                    m_hwNode ;
    DlsDevice                *q_ptr = nullptr;

private:
    QMap<QString, int> mLastCpuStat;
    double mCpuUsage;

    QStringList  attrisMouse;
    QStringList  attrisCPU;
    QStringList  attrisStorage;
    QStringList  attrisGPU;
    QStringList  attrisMemory;
    QStringList  attrisBios;
    QStringList  attrisBluetooth;
    QStringList  attrisAudio;
    QStringList  attrisNetwork;
    QStringList  attrisCameraImage;
    QStringList  attrisKeyboard;
    QStringList  attrisComputer;
    QStringList  attrisOthers;

};

DDEVICE_END_NAMESPACE

#endif
