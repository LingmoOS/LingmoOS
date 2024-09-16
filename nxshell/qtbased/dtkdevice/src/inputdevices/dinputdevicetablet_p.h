// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICETABLET_P_H
#define DINPUTDEVICETABLET_P_H
#include "dinputdevicepointer_p.h"
#include "dinputdevicetablet.h"
#include "wacominterface.h"
DDEVICE_BEGIN_NAMESPACE
class LIBDTKDEVICESHARED_EXPORT DInputDeviceTabletPrivate : public DInputDevicePointerPrivate
{
public:
    DInputDeviceTabletPrivate(DInputDeviceTablet *q);
    ~DInputDeviceTabletPrivate() override;

private:
    WacomInterface *m_wacomInter;
    D_DECLARE_PUBLIC(DInputDeviceTablet)
};
DDEVICE_END_NAMESPACE
#endif  // DINPUTDEVICETABLET_P_H
