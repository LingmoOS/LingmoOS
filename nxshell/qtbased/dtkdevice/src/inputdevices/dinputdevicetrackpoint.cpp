// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicetrackpoint.h"
#include "dinputdevicetrackpoint_p.h"

DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DUnexpected;

DInputDeviceTrackPoint::DInputDeviceTrackPoint(const DeviceInfo &info, bool enabled, QObject *parent)
    : DInputDevicePointer(*new DInputDeviceTrackPointPrivate(this), info, enabled, parent)
{
    D_D(DInputDeviceTrackPoint);
    connect(d->m_trackPointInter,
            &TrackPointInterface::MiddleButtonEnabledChanged,
            this,
            &DInputDeviceTrackPoint::middleButtonEnabledChanged);
    connect(d->m_trackPointInter,
            &TrackPointInterface::MiddleButtonTimeoutChanged,
            this,
            &DInputDeviceTrackPoint::middleButtonTimeoutChanged);
    connect(
        d->m_trackPointInter, &TrackPointInterface::WheelEmulationChanged, this, &DInputDeviceTrackPoint::wheelEmulationChanged);
    connect(d->m_trackPointInter,
            &TrackPointInterface::WheelEmulationButtonChanged,
            this,
            &DInputDeviceTrackPoint::wheelEmulationButtonChanged);
    connect(d->m_trackPointInter,
            &TrackPointInterface::WheelEmulationTimeoutChanged,
            this,
            &DInputDeviceTrackPoint::wheelEmulationTimeoutChanged);
    connect(d->m_trackPointInter,
            &TrackPointInterface::WheelHorizScrollChanged,
            this,
            &DInputDeviceTrackPoint::wheelHorizontalScrollChanged);
}

DInputDeviceTrackPoint::~DInputDeviceTrackPoint() = default;

DInputDeviceTrackPointPrivate::DInputDeviceTrackPointPrivate(DInputDeviceTrackPoint *q)
    : DInputDevicePointerPrivate(q)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("org.deepin.dtk.InputDevices");
#else
    const QString &Service = QStringLiteral("com.deepin.daemon.InputDevices");
#endif
    m_trackPointInter = new TrackPointInterface(Service);
}

DInputDeviceTrackPointPrivate::~DInputDeviceTrackPointPrivate()
{
    delete m_trackPointInter;
}

bool DInputDeviceTrackPoint::middleButtonEnabled() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->MiddleButtonEnabled();
}

qint32 DInputDeviceTrackPoint::middleButtonTimeout() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->MiddleButtonTimeout();
}

bool DInputDeviceTrackPoint::wheelEmulation() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->WheelEmulation();
}

qint32 DInputDeviceTrackPoint::wheelEmulationButton() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->WheelEmulationButton();
}

qint32 DInputDeviceTrackPoint::wheelEmulationTimeout() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->WheelEmulationTimeout();
}

bool DInputDeviceTrackPoint::wheelHorizontalScroll() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->WheelHorizScroll();
}

void DInputDeviceTrackPoint::setMiddleButtonEnabled(bool enabled)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetMiddleButtonEnabled(enabled);
}

void DInputDeviceTrackPoint::setMiddleButtonTimeout(qint32 timeout)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetMiddleButtonTimeout(timeout);
}

void DInputDeviceTrackPoint::setWheelEmulation(bool emulation)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetWheelEmulation(emulation);
}

void DInputDeviceTrackPoint::setWheelEmulationButton(qint32 emulationButton)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetWheelEmulationButton(emulationButton);
}

void DInputDeviceTrackPoint::setWheelEmulationTimeout(qint32 timeout)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetWheelEmulationTimeout(timeout);
}

void DInputDeviceTrackPoint::setWheelHorizontalScroll(bool horizontal)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetWheelHorizScroll(horizontal);
}

DExpected<void> DInputDeviceTrackPoint::reset()
{
    D_D(DInputDeviceTrackPoint);
    auto reply = d->m_trackPointInter->Reset();
    reply.waitForFinished();
    if (!reply.isValid()) {
        return DUnexpected<>{DError{reply.error().type(), reply.error().message()}};
    } else {
        return {};
    }
}

bool DInputDeviceTrackPoint::leftHanded() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->LeftHanded();
}

void DInputDeviceTrackPoint::setLeftHanded(bool leftHanded)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetLeftHanded(leftHanded);
}

AccelerationProfile DInputDeviceTrackPoint::accelerationProfile() const
{
    // TODO(asterwyx): Here we cannot acquire the profile using dde-daemon
    return AccelerationProfile::Adaptive;
}

void DInputDeviceTrackPoint::setAccelerationProfile(AccelerationProfile profile)
{
    Q_UNUSED(profile)
}

ScrollMethod DInputDeviceTrackPoint::scrollMethod() const
{
    return ScrollMethod::ScrollOnButtonDown;
}

void DInputDeviceTrackPoint::setScrollMethod(ScrollMethod method)
{
    Q_UNUSED(method)
}

double DInputDeviceTrackPoint::accelerationSpeed() const
{
    D_DC(DInputDeviceTrackPoint);
    return d->m_trackPointInter->MotionAcceleration();
}

void DInputDeviceTrackPoint::setAccelerationSpeed(double speed)
{
    D_D(DInputDeviceTrackPoint);
    d->m_trackPointInter->SetMotionAcceleration(speed);
}
DDEVICE_END_NAMESPACE
