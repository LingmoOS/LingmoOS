// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicekeyboard.h"
#include "dinputdevicekeyboard_p.h"
#include "dtkdevice_global.h"
DDEVICE_BEGIN_NAMESPACE

DInputDeviceKeyboardPrivate::DInputDeviceKeyboardPrivate(DInputDeviceKeyboard *q)
    : DInputDeviceGenericPrivate(q)
{
}

DInputDeviceKeyboardPrivate::~DInputDeviceKeyboardPrivate() = default;

DInputDeviceKeyboard::DInputDeviceKeyboard(const DeviceInfo &info, bool enabled, QObject *parent)
    : DInputDeviceGeneric(*new DInputDeviceKeyboardPrivate(this), info, enabled, parent)
{
}

DExpected<void> DInputDeviceKeyboard::reset()
{
    return {};
}

DInputDeviceKeyboard::~DInputDeviceKeyboard() = default;
DDEVICE_END_NAMESPACE
