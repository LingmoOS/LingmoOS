// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkdevice_global.h"

#include "dinputdevicegeneric_p.h"
#include "dinputdevicepointer.h"
DDEVICE_BEGIN_NAMESPACE

class LIBDTKDEVICESHARED_EXPORT DInputDevicePointerPrivate : public DInputDeviceGenericPrivate
{
public:
    explicit DInputDevicePointerPrivate(DInputDevicePointer *q);

private:
    D_DECLARE_PUBLIC(DInputDevicePointer)
};

DDEVICE_END_NAMESPACE
