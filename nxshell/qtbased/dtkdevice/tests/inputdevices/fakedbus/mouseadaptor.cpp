// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mouseadaptor.h"
#include <QtDBus>
DDEVICE_BEGIN_NAMESPACE

MouseAdaptor::MouseAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_adaptiveAccelProfile(AdaptiveAccelProfileDefault)
    , m_disableTpad(DisableTpadDefault)
    , m_exist(ExistDefault)
    , m_leftHanded(LeftHandedDefault)
    , m_middleButtonEmulation(MiddleButtonEmulationDefault)
    , m_naturalScroll(NaturalScrollDefault)
    , m_motionAcceleration(MotionAccelerationDefault)
    , m_doubleClick(DoubleClickDefault)
    , m_dragThreshold(DragThresholdDefault)
    , m_deviceList(DeviceListDefault)
{
    autoRelaySignals();
}

MouseAdaptor::~MouseAdaptor() {}

bool MouseAdaptor::AdaptiveAccelProfile() const
{
    return m_adaptiveAccelProfile;
}

bool MouseAdaptor::DisableTpad() const
{
    return m_disableTpad;
}

bool MouseAdaptor::Exist() const
{
    return m_exist;
}

bool MouseAdaptor::LeftHanded() const
{
    return m_leftHanded;
}

bool MouseAdaptor::MiddleButtonEmulation() const
{
    return m_middleButtonEmulation;
}

bool MouseAdaptor::NaturalScroll() const
{
    return m_naturalScroll;
}

double MouseAdaptor::MotionAcceleration() const
{
    return m_motionAcceleration;
}

qint32 MouseAdaptor::DoubleClick() const
{
    return m_doubleClick;
}

qint32 MouseAdaptor::DragThreshold() const
{
    return m_dragThreshold;
}

QString MouseAdaptor::DeviceList() const
{
    return m_deviceList;
}

void MouseAdaptor::SetAdaptiveAccelProfile(bool adaptive)
{
    m_adaptiveAccelProfile = adaptive;
    Q_EMIT this->AdaptiveAccelProfileChanged(adaptive);
}

void MouseAdaptor::SetDisableTpad(bool disable)
{
    m_disableTpad = disable;
    Q_EMIT this->DisableTpadChanged(disable);
}

void MouseAdaptor::SetLeftHanded(bool leftHanded)
{
    m_leftHanded = leftHanded;
    Q_EMIT this->LeftHandedChanged(leftHanded);
}

void MouseAdaptor::SetMiddleButtonEmulation(bool emulation)
{
    m_middleButtonEmulation = emulation;
    Q_EMIT this->MiddleButtonEmulationChanged(emulation);
}

void MouseAdaptor::SetNaturalScroll(bool naturalScroll)
{
    m_naturalScroll = naturalScroll;
    Q_EMIT this->NaturalScrollChanged(naturalScroll);
}

void MouseAdaptor::SetMotionAcceleration(double acceleration)
{
    m_motionAcceleration = acceleration;
    Q_EMIT this->MotionAccelerationChanged(acceleration);
}

void MouseAdaptor::SetDoubleClick(qint32 interval)
{
    m_doubleClick = interval;
    Q_EMIT this->DoubleClickChanged(interval);
}

void MouseAdaptor::SetDragThreshold(qint32 threshold)
{
    m_dragThreshold = threshold;
    Q_EMIT this->DragThresholdChanged(threshold);
}

void MouseAdaptor::Reset()
{
    m_adaptiveAccelProfile = AdaptiveAccelProfileDefault;
    m_disableTpad = DisableTpadDefault;
    m_exist = ExistDefault;
    m_leftHanded = LeftHandedDefault;
    m_middleButtonEmulation = MiddleButtonEmulationDefault;
    m_naturalScroll = NaturalScrollDefault;
    m_motionAcceleration = MotionAccelerationDefault;
    m_doubleClick = DoubleClickDefault;
    m_dragThreshold = DragThresholdDefault;
    m_deviceList = DeviceListDefault;
}

DDEVICE_END_NAMESPACE
