// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEMOUSE_P_H
#define DINPUTDEVICEMOUSE_P_H

#include "dinputdevicemouse.h"
#include "dinputdevicepointer_p.h"
#include "mouseinterface.h"

DDEVICE_BEGIN_NAMESPACE

class LIBDTKDEVICESHARED_EXPORT DInputDeviceMousePrivate : public DInputDevicePointerPrivate
{
public:
    explicit DInputDeviceMousePrivate(DInputDeviceMouse *q);
    ~DInputDeviceMousePrivate() override;

private:
    MouseInterface *m_mouseInter;
    D_DECLARE_PUBLIC(DInputDeviceMouse)
};

DDEVICE_END_NAMESPACE

#endif  // DINPUTDEVICEMOUSE_P_H
