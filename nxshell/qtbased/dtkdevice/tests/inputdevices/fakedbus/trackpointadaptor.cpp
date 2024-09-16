// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "trackpointadaptor.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE

TrackPointAdaptor::TrackPointAdaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_exist(ExistDefault)
    , m_leftHanded(LeftHandedDefault)
    , m_middleButtonEnabled(MiddleButtonEnabledDefault)
    , m_wheelEmulation(WheelEmulationDefault)
    , m_wheelHorizScroll(WheelHorizScrollDefault)
    , m_motionAcceleration(MotionAccelerationDefault)
    , m_middleButtonTimeout(MiddleButtonTimeoutDefault)
    , m_wheelEmulationButton(WheelEmulationButtonDefault)
    , m_wheelEmulationTimeout(WheelEmulationTimeoutDefault)
    , m_deviceList(DeviceListDefault)
{
    autoRelaySignals();
}

TrackPointAdaptor::~TrackPointAdaptor() {}

bool TrackPointAdaptor::Exist() const
{
    return m_exist;
}

bool TrackPointAdaptor::LeftHanded() const
{
    return m_leftHanded;
}

bool TrackPointAdaptor::MiddleButtonEnabled() const
{
    return m_middleButtonEnabled;
}

bool TrackPointAdaptor::WheelEmulation() const
{
    return m_wheelEmulation;
}

bool TrackPointAdaptor::WheelHorizScroll() const
{
    return m_wheelHorizScroll;
}

double TrackPointAdaptor::MotionAcceleration() const
{
    return m_motionAcceleration;
}

qint32 TrackPointAdaptor::MiddleButtonTimeout() const
{
    return m_middleButtonTimeout;
}

qint32 TrackPointAdaptor::WheelEmulationButton() const
{
    return m_wheelEmulationButton;
}

qint32 TrackPointAdaptor::WheelEmulationTimeout() const
{
    return m_wheelEmulationTimeout;
}

QString TrackPointAdaptor::DeviceList() const
{
    return m_deviceList;
}

void TrackPointAdaptor::SetLeftHanded(bool leftHanded)
{
    m_leftHanded = leftHanded;
    Q_EMIT this->LeftHandedChanged(leftHanded);
}

void TrackPointAdaptor::SetMiddleButtonEnabled(bool enabled)
{
    m_middleButtonEnabled = enabled;
    Q_EMIT this->MiddleButtonEnabledChanged(enabled);
}

void TrackPointAdaptor::SetWheelEmulation(bool emulation)
{
    m_wheelEmulation = emulation;
    Q_EMIT this->WheelEmulationChanged(emulation);
}

void TrackPointAdaptor::SetWheelHorizScroll(bool horizScroll)
{
    m_wheelHorizScroll = horizScroll;
    Q_EMIT this->WheelHorizScrollChanged(horizScroll);
}

void TrackPointAdaptor::SetMotionAcceleration(double acceleration)
{
    m_motionAcceleration = acceleration;
    Q_EMIT this->MotionAccelerationChanged(acceleration);
}

void TrackPointAdaptor::SetMiddleButtonTimeout(qint32 timeout)
{
    m_middleButtonTimeout = timeout;
    Q_EMIT this->MiddleButtonTimeoutChanged(timeout);
}

void TrackPointAdaptor::SetWheelEmulationButton(qint32 button)
{
    m_wheelEmulationButton = button;
    Q_EMIT this->WheelEmulationButtonChanged(button);
}

void TrackPointAdaptor::SetWheelEmulationTimeout(qint32 timeout)
{
    m_wheelEmulationTimeout = timeout;
    Q_EMIT this->WheelEmulationTimeoutChanged(timeout);
}

void TrackPointAdaptor::Reset()
{
    m_exist = ExistDefault;
    m_leftHanded = LeftHandedDefault;
    m_middleButtonEnabled = MiddleButtonEnabledDefault;
    m_wheelEmulation = WheelEmulationDefault;
    m_wheelHorizScroll = WheelHorizScrollDefault;
    m_motionAcceleration = MotionAccelerationDefault;
    m_middleButtonTimeout = MiddleButtonTimeoutDefault;
    m_wheelEmulationButton = WheelEmulationButtonDefault;
    m_wheelEmulationTimeout = WheelEmulationTimeoutDefault;
    m_deviceList = DeviceListDefault;
}

DDEVICE_END_NAMESPACE
