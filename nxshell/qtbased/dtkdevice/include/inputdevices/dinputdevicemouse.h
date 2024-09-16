// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICEMOUSE_H
#define DINPUTDEVICEMOUSE_H

#include "dtkdevice_global.h"

#include <DExpected>
#include <QObject>

#include "dinputdevicepointer.h"

DDEVICE_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DExpected;
class DInputDeviceMousePrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceMouse : public DInputDevicePointer
{
    Q_OBJECT
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)
    Q_PROPERTY(
        bool middleButtonEmulation READ middleButtonEmulation WRITE setMiddleButtonEmulation NOTIFY middleButtonEmulationChanged)

public:
    ~DInputDeviceMouse() override;
    bool naturalScroll() const;
    bool middleButtonEmulation() const;

    void setNaturalScroll(bool naturalScroll);
    void setMiddleButtonEmulation(bool middleButtonEmulation);

    bool leftHanded() const override;
    ScrollMethod scrollMethod() const override;
    AccelerationProfile accelerationProfile() const override;
    double accelerationSpeed() const override;
    void setLeftHanded(bool leftHanded) override;
    void setScrollMethod(ScrollMethod method) override;
    void setAccelerationProfile(AccelerationProfile profile) override;
    void setAccelerationSpeed(double speed) override;

Q_SIGNALS:
    void naturalScrollChanged(bool naturalScroll);
    void middleButtonEmulationChanged(bool emulation);

public Q_SLOTS:
    DExpected<void> reset() override;

protected:
    explicit DInputDeviceMouse(const DeviceInfo &info = DeviceInfoInitializer, bool enabled = true, QObject *parent = nullptr);

private:
    friend class DInputDeviceManager;
    D_DECLARE_PRIVATE(DInputDeviceMouse)
};

DDEVICE_END_NAMESPACE

#endif  // DINPUTDEVICEMOUSE_H
