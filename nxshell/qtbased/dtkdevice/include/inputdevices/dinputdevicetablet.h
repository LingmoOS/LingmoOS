// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICETABLET_H
#define DINPUTDEVICETABLET_H
#include "dinputdevicepointer.h"
#include "dtkdevice_global.h"
#include <DExpected>

DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
class DInputDeviceTabletPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceTablet : public DInputDevicePointer
{
    Q_OBJECT
    Q_PROPERTY(bool cursorMode READ cursorMode WRITE setCursorMode NOTIFY cursorModeChanged)

public:
    bool cursorMode() const;
    void setCursorMode(bool cursorMode);
    ~DInputDeviceTablet() override;

    bool leftHanded() const override;
    ScrollMethod scrollMethod() const override;
    AccelerationProfile accelerationProfile() const override;
    double accelerationSpeed() const override;
    void setLeftHanded(bool leftHanded) override;
    void setScrollMethod(ScrollMethod scrollMethod) override;
    void setAccelerationProfile(AccelerationProfile accelerationProfile) override;
    void setAccelerationSpeed(double accelerationSpeed) override;

Q_SIGNALS:
    void cursorModeChanged(bool cursorMode);

public Q_SLOTS:
    DExpected<void> reset() override;

protected:
    DInputDeviceTablet(const DeviceInfo &info = DeviceInfoInitializer, bool enabled = true, QObject *parent = nullptr);

private:
    friend class DInputDeviceManager;
    D_DECLARE_PRIVATE(DInputDeviceTablet);
};
DDEVICE_END_NAMESPACE
#endif  // DINPUTDEVICETABLET_H
