// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicemouse.h"
#include "dinputdevicemouse_p.h"
#include "dtkinputdevices_types.h"

DDEVICE_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DInputDeviceMousePrivate::DInputDeviceMousePrivate(DInputDeviceMouse *q)
    : DInputDevicePointerPrivate(q)
{
#if defined(USE_FAKE_INTERFACE)
    const QString &Service = QStringLiteral("org.deepin.dtk.InputDevices");
#else
    const QString &Service = QStringLiteral("com.deepin.daemon.InputDevices");
#endif
    m_mouseInter = new MouseInterface(Service);
}

DInputDeviceMousePrivate::~DInputDeviceMousePrivate()
{
    delete m_mouseInter;
}

DInputDeviceMouse::DInputDeviceMouse(const DeviceInfo &info, bool enabled, QObject *parent)
    : DInputDevicePointer(*new DInputDeviceMousePrivate(this), info, enabled, parent)
{
    D_D(DInputDeviceMouse);
    connect(d->m_mouseInter, &MouseInterface::NaturalScrollChanged, this, &DInputDeviceMouse::naturalScrollChanged);
    connect(
        d->m_mouseInter, &MouseInterface::MiddleButtonEmulationChanged, this, &DInputDeviceMouse::middleButtonEmulationChanged);
}

DInputDeviceMouse::~DInputDeviceMouse() = default;

bool DInputDeviceMouse::leftHanded() const
{
    D_DC(DInputDeviceMouse);
    return d->m_mouseInter->LeftHanded();
}

void DInputDeviceMouse::setLeftHanded(bool leftHanded)
{
    D_D(DInputDeviceMouse);
    d->m_mouseInter->SetLeftHanded(leftHanded);
}

ScrollMethod DInputDeviceMouse::scrollMethod() const
{
    return ScrollMethod::ScrollOnButtonDown;
}

void DInputDeviceMouse::setScrollMethod(ScrollMethod method)
{
    Q_UNUSED(method)
}

AccelerationProfile DInputDeviceMouse::accelerationProfile() const
{
    D_DC(DInputDeviceMouse);
    if (d->m_mouseInter->AdaptiveAccelProfile()) {
        return AccelerationProfile::Adaptive;
    } else {
        return AccelerationProfile::Flat;
    }
}

void DInputDeviceMouse::setAccelerationProfile(AccelerationProfile profile)
{
    D_D(DInputDeviceMouse);
    if (profile == AccelerationProfile::Adaptive) {
        d->m_mouseInter->SetAdaptiveAccelProfile(true);
    } else if (profile == AccelerationProfile::Flat) {
        d->m_mouseInter->SetAdaptiveAccelProfile(false);
    } else {
        qWarning() << "Cannot apply acceleration profile none to mouse.";
    }
}

double DInputDeviceMouse::accelerationSpeed() const
{
    D_DC(DInputDeviceMouse);
    return d->m_mouseInter->MotionAcceleration();
}

void DInputDeviceMouse::setAccelerationSpeed(double speed)
{
    D_D(DInputDeviceMouse);
    d->m_mouseInter->SetMotionAcceleration(speed);
}

bool DInputDeviceMouse::naturalScroll() const
{
    D_DC(DInputDeviceMouse);
    return d->m_mouseInter->NaturalScroll();
}

bool DInputDeviceMouse::middleButtonEmulation() const
{
    D_DC(DInputDeviceMouse);
    return d->m_mouseInter->MiddleButtonEmulation();
}

DExpected<void> DInputDeviceMouse::reset()
{
    D_D(DInputDeviceMouse);
    auto result = d->m_mouseInter->Reset();
    if (!result.isValid()) {
        return DUnexpected<>{DError{result.error().type(), result.error().message()}};
    } else {
        return {};
    }
}

void DInputDeviceMouse::setNaturalScroll(bool naturalScroll)
{
    D_D(DInputDeviceMouse);
    d->m_mouseInter->SetNaturalScroll(naturalScroll);
}

void DInputDeviceMouse::setMiddleButtonEmulation(bool middleButtonEmulation)
{
    D_D(DInputDeviceMouse);
    d->m_mouseInter->SetMiddleButtonEmulation(middleButtonEmulation);
}

DDEVICE_END_NAMESPACE
