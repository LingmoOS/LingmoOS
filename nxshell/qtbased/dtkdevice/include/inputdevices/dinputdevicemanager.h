// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEMANAGER_H
#define DINPUTDEVICEMANAGER_H

#include "dtkdevice_global.h"
#include <DExpected>
#include <DObject>
#include <QObject>

#include "dtkinputdevices_types.h"

DDEVICE_BEGIN_NAMESPACE
class DInputDeviceGeneric;
class DInputDeviceSetting;
DDEVICE_END_NAMESPACE
using DInputDevicePtr = QSharedPointer<DTK_DEVICE_NAMESPACE::DInputDeviceGeneric>;
using DInputSettingPtr = QSharedPointer<DTK_DEVICE_NAMESPACE::DInputDeviceSetting>;
using DInputDeviceInfoList = QList<DTK_DEVICE_NAMESPACE::DeviceInfo>;
DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DObject;
class DInputDeviceManagerPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceManager : public QObject, public DObject
{
    Q_OBJECT
    Q_PROPERTY(DInputDeviceInfoList deviceInfos READ deviceInfos)

public:
    explicit DInputDeviceManager(QObject *parent = nullptr);
    ~DInputDeviceManager() override;
    DInputDeviceInfoList deviceInfos() const;

Q_SIGNALS:
    void deviceAdded(const DeviceInfo &device);
    void deviceRemoved(const DeviceInfo &device);

public Q_SLOTS:
    DExpected<DInputDevicePtr> createDevice(const DeviceInfo &info);
    DExpected<DInputSettingPtr> setting();

private:
    template <DeviceType deviceType> void handleDeviceChanged(const QString &deviceList);
    D_DECLARE_PRIVATE(DInputDeviceManager)
};

DDEVICE_END_NAMESPACE
#endif
