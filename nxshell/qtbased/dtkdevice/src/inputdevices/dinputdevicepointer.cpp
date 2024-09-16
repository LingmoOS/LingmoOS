// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicepointer_p.h"

DDEVICE_BEGIN_NAMESPACE

DInputDevicePointerPrivate::DInputDevicePointerPrivate(DInputDevicePointer *q)
    : DInputDeviceGenericPrivate(q)
{
}

DInputDevicePointer::DInputDevicePointer(const DeviceInfo &info, bool enabled, QObject *parent)
    : DInputDevicePointer(*new DInputDevicePointerPrivate(this), info, enabled, parent)
{
}

DInputDevicePointer::DInputDevicePointer(DInputDevicePointerPrivate &dd, const DeviceInfo &info, bool enabled, QObject *parent)
    : DInputDeviceGeneric(dd, info, enabled, parent)
{
}

DInputDevicePointer::~DInputDevicePointer() = default;

DDEVICE_END_NAMESPACE
