// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEKEYBOARD_P_H
#define DINPUTDEVICEKEYBOARD_P_H
#include "dinputdevicegeneric_p.h"
#include "dinputdevicekeyboard.h"
OCEANVICE_BEGIN_NAMESPACE
class LIBDTKDEVICESHARED_EXPORT DInputDeviceKeyboardPrivate : public DInputDeviceGenericPrivate
{
public:
    DInputDeviceKeyboardPrivate(DInputDeviceKeyboard *q);
    ~DInputDeviceKeyboardPrivate() override;

private:
    D_DECLARE_PUBLIC(DInputDeviceKeyboard)
};
OCEANVICE_END_NAMESPACE

#endif  // DINPUTDEVICEKEYBOARD_P_H
