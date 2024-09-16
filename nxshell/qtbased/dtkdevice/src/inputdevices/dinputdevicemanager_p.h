// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEMANAGER_P_H
#define DINPUTDEVICEMANAGER_P_H

#include "dinputdevicemanager.h"
#include "mouseinterface.h"
#include "touchpadinterface.h"
#include "trackpointinterface.h"
#include "wacominterface.h"
#include <DObjectPrivate>

DDEVICE_BEGIN_NAMESPACE
class LIBDTKDEVICESHARED_EXPORT DInputDeviceManagerPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    explicit DInputDeviceManagerPrivate(DInputDeviceManager *q);
    ~DInputDeviceManagerPrivate() override;
    void initializeDeviceInfos();
    void addDevice(const DeviceInfo &info);
    void removeDevice(const DeviceInfo &info);
    void removeDeviceById(quint32 id);

private:
    MouseInterface *m_mouseInter;
    TouchPadInterface *m_touchPadInter;
    TrackPointInterface *m_trackPointInter;
    WacomInterface *m_wacomInter;
    QList<DeviceInfo> m_deviceInfos;
    QMap<DeviceType, QList<quint32>> m_idMap;
    D_DECLARE_PUBLIC(DInputDeviceManager)
};

DDEVICE_END_NAMESPACE
#endif
