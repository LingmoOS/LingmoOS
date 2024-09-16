// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEGENERIC_P_H
#define DINPUTDEVICEGENERIC_P_H
#include "dinputdevicegeneric.h"

#include <DObjectPrivate>

DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DObjectPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceGenericPrivate : public DObjectPrivate
{
public:
    explicit DInputDeviceGenericPrivate(DInputDeviceGeneric *q);

private:
    DeviceInfo m_info;
    bool m_enabled;
    D_DECLARE_PUBLIC(DInputDeviceGeneric)
};
DDEVICE_END_NAMESPACE

#endif
