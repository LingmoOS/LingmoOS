// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicetouchpad.h"
#include "dinputdevicetouchpad_p.h"
#include "touchpadinterface.h"

DDEVICE_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DError;
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DUnexpected;

DInputDeviceTouchPadPrivate::DInputDeviceTouchPadPrivate(DInputDeviceTouchPad *q)
    : DInputDevicePointerPrivate(q)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("org.deepin.dtk.InputDevices");
#else
    const QString &Service = QStringLiteral("com.deepin.daemon.InputDevices");
#endif
    m_touchPadInter = new TouchPadInterface(Service);
}

DInputDeviceTouchPadPrivate::~DInputDeviceTouchPadPrivate()
{
    delete m_touchPadInter;
}

DInputDeviceTouchPad::DInputDeviceTouchPad(const DeviceInfo &info, bool enabled, QObject *parent)
    : DInputDevicePointer(*new DInputDeviceTouchPadPrivate(this), info, enabled, parent)
{
    D_D(DInputDeviceTouchPad);
    connect(
        d->m_touchPadInter, &TouchPadInterface::DisableIfTypingChanged, this, &DInputDeviceTouchPad::disableWhileTypingChanged);
    connect(d->m_touchPadInter, &TouchPadInterface::NaturalScrollChanged, this, &DInputDeviceTouchPad::naturalScrollChanged);
    connect(d->m_touchPadInter, &TouchPadInterface::TapClickChanged, this, &DInputDeviceTouchPad::tapToClickChanged);
    this->setEnabled(this->enabled());
}

DInputDeviceTouchPad::~DInputDeviceTouchPad() = default;

bool DInputDeviceTouchPad::disableWhileTyping() const
{
    D_DC(DInputDeviceTouchPad);
    return d->m_touchPadInter->DisableIfTyping();
}

bool DInputDeviceTouchPad::naturalScroll() const
{
    D_DC(DInputDeviceTouchPad);
    return d->m_touchPadInter->NaturalScroll();
}

bool DInputDeviceTouchPad::tapToClick() const
{
    D_DC(DInputDeviceTouchPad);
    return d->m_touchPadInter->TapClick();
}

DExpected<void> DInputDeviceTouchPad::reset()
{
    D_D(DInputDeviceTouchPad);
    auto result = d->m_touchPadInter->Reset();
    result.waitForFinished();
    if (!result.isValid()) {
        return DUnexpected<>{DError{result.error().type(), result.error().message()}};
    } else {
        return {};
    }
}

bool DInputDeviceTouchPad::enabled() const
{
    D_DC(DInputDeviceTouchPad);
    return d->m_touchPadInter->TPadEnable();
}

DExpected<void> DInputDeviceTouchPad::enable(bool enabled)
{
    D_D(DInputDeviceTouchPad);
    d->m_touchPadInter->SetTPadEnable(enabled);
    DInputDeviceGeneric::setEnabled(enabled);
    return {};
}

void DInputDeviceTouchPad::setDisableWhileTyping(bool disabled)
{
    D_D(DInputDeviceTouchPad);
    d->m_touchPadInter->SetDisableIfTyping(disabled);
}

void DInputDeviceTouchPad::setNaturalScroll(bool naturalScroll)
{
    D_D(DInputDeviceTouchPad);
    d->m_touchPadInter->SetNaturalScroll(naturalScroll);
}

void DInputDeviceTouchPad::setTapToClick(bool tapToClick)
{
    D_D(DInputDeviceTouchPad);
    d->m_touchPadInter->SetTapClick(tapToClick);
}

bool DInputDeviceTouchPad::leftHanded() const
{
    D_DC(DInputDeviceTouchPad);
    return d->m_touchPadInter->LeftHanded();
}

void DInputDeviceTouchPad::setLeftHanded(bool leftHanded)
{
    D_D(DInputDeviceTouchPad);
    d->m_touchPadInter->SetLeftHanded(leftHanded);
}

ScrollMethod DInputDeviceTouchPad::scrollMethod() const
{
    D_DC(DInputDeviceTouchPad);
    if (d->m_touchPadInter->EdgeScroll()) {
        return ScrollMethod::ScrollEdge;
    } else if (d->m_touchPadInter->VertScroll()) {
        return ScrollMethod::ScrollTwoFinger;
    } else {
        return ScrollMethod::NoScroll;
    }
}

void DInputDeviceTouchPad::setScrollMethod(ScrollMethod method)
{
    D_D(DInputDeviceTouchPad);
    switch (method) {
        case ScrollMethod::ScrollEdge:
            d->m_touchPadInter->SetEdgeScroll(true);
            d->m_touchPadInter->SetVertScroll(false);
            break;
        case ScrollMethod::ScrollTwoFinger:
            d->m_touchPadInter->SetEdgeScroll(false);
            d->m_touchPadInter->SetVertScroll(true);
            break;
        case ScrollMethod::ScrollOnButtonDown:
            break;
        case ScrollMethod::NoScroll:
            d->m_touchPadInter->SetEdgeScroll(false);
            d->m_touchPadInter->SetVertScroll(false);
            break;
        default:
            Q_UNREACHABLE();
    }
}

AccelerationProfile DInputDeviceTouchPad::accelerationProfile() const
{
    return AccelerationProfile::Na;
}

void DInputDeviceTouchPad::setAccelerationProfile(AccelerationProfile profile)
{
    Q_UNUSED(profile)
}

double DInputDeviceTouchPad::accelerationSpeed() const
{
    return 0;
}

void DInputDeviceTouchPad::setAccelerationSpeed(double speed)
{
    Q_UNUSED(speed)
}
DDEVICE_END_NAMESPACE
