// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include "dinputdevicepointer_p.h"
#include "dinputdevicetouchpad.h"
#include "touchpadinterface.h"

DDEVICE_BEGIN_NAMESPACE

class LIBDTKDEVICESHARED_EXPORT DInputDeviceTouchPadPrivate : public DInputDevicePointerPrivate
{
public:
    explicit DInputDeviceTouchPadPrivate(DInputDeviceTouchPad *q);
    ~DInputDeviceTouchPadPrivate();

private:
    TouchPadInterface *m_touchPadInter;
    D_DECLARE_PUBLIC(DInputDeviceTouchPad)
};

DDEVICE_END_NAMESPACE
