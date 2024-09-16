// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEKEYBOARD_H
#define DINPUTDEVICEKEYBOARD_H
#include "dinputdevicegeneric.h"
#include "dtkdevice_global.h"
DDEVICE_BEGIN_NAMESPACE
class DInputDeviceKeyboardPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceKeyboard : public DInputDeviceGeneric
{
    Q_OBJECT

public:
    ~DInputDeviceKeyboard() override;

public Q_SLOTS:
    DExpected<void> reset() override;

protected:
    DInputDeviceKeyboard(const DeviceInfo &info = DeviceInfoInitializer, bool enabled = true, QObject *parent = nullptr);

private:
    friend class DInputDeviceManager;
    D_DECLARE_PRIVATE(DInputDeviceKeyboard)
};

DDEVICE_END_NAMESPACE
#endif  // DINPUTDEVICEKEYBOARD_H
