// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "touchpadservice.h"
#include <QtDBus>

DDEVICE_BEGIN_NAMESPACE
TouchPadService::TouchPadService(const QString &service, const QString &path, QObject *parent)
    : QObject(parent)
    , m_disableIfTyping(DisableIfTypingDefault)
    , m_edgeScroll(EdgeScrollDefault)
    , m_exist(ExistDefault)
    , m_leftHanded(LeftHandedDefault)
    , m_naturalScroll(NaturalScrollDefault)
    , m_tPadEnable(TPadEnableDefault)
    , m_tapClick(TapClickDefault)
    , m_vertScroll(VertScrollDefault)
    , m_motionAcceleration(MotionAccelerationDefault)
    , m_deltaScroll(DeltaScrollDefault)
    , m_doubleClick(DoubleClickDefault)
    , m_dragThreshold(DragThresholdDefault)
    , m_deviceList(DeviceListDefault)
    , m_service(service)
    , m_path(path)
{
    registerService(m_service, m_path);
}

TouchPadService::~TouchPadService()
{
    unregisterService();
}

bool TouchPadService::registerService(const QString &service, const QString &path)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService(service)) {
        QString errorMsg = connection.lastError().message();
        if (errorMsg.isEmpty())
            errorMsg = "maybe it's running";

        qWarning() << QString("Can't register the %1 service, %2.").arg(service).arg(errorMsg);
        return false;
    }
    if (!connection.registerObject(path, this, QDBusConnection::ExportAllContents)) {
        qWarning() << QString("Can't register %1 the D-Bus object.").arg(path);
        return false;
    }
    return true;
}

void TouchPadService::unregisterService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.unregisterObject(m_path);
    connection.unregisterService(m_service);
}

bool TouchPadService::DisableIfTyping() const
{
    return m_disableIfTyping;
}

bool TouchPadService::EdgeScroll() const
{
    return m_edgeScroll;
}

bool TouchPadService::Exist() const
{
    return m_exist;
}

bool TouchPadService::LeftHanded() const
{
    return m_leftHanded;
}

bool TouchPadService::TapClick() const
{
    return m_tapClick;
}

bool TouchPadService::NaturalScroll() const
{
    return m_naturalScroll;
}

bool TouchPadService::TPadEnable() const
{
    return m_tPadEnable;
}

bool TouchPadService::VertScroll() const
{
    return m_vertScroll;
}

double TouchPadService::MotionAcceleration() const
{
    return m_motionAcceleration;
}

qint32 TouchPadService::DeltaScroll() const
{
    return m_deltaScroll;
}

qint32 TouchPadService::DoubleClick() const
{
    return m_doubleClick;
}

qint32 TouchPadService::DragThreshold() const
{
    return m_dragThreshold;
}

QString TouchPadService::DeviceList() const
{
    return m_deviceList;
}

void TouchPadService::SetDisableIfTyping(bool disable)
{
    m_disableIfTyping = disable;
    Q_EMIT this->DisableIfTypingChanged(disable);
}

void TouchPadService::SetEdgeScroll(bool edgeScroll)
{
    m_edgeScroll = edgeScroll;
    Q_EMIT this->EdgeScrollChanged(edgeScroll);
}

void TouchPadService::SetLeftHanded(bool leftHanded)
{
    m_leftHanded = leftHanded;
    Q_EMIT this->LeftHandedChanged(leftHanded);
}

void TouchPadService::SetNaturalScroll(bool naturalScroll)
{
    m_naturalScroll = naturalScroll;
    Q_EMIT this->NaturalScrollChanged(naturalScroll);
}

void TouchPadService::SetTPadEnable(bool enable)
{
    m_tPadEnable = enable;
    Q_EMIT this->TPadEnableChanged(enable);
}

void TouchPadService::SetTapClick(bool tapClick)
{
    m_tapClick = tapClick;
    Q_EMIT this->TapClickChanged(tapClick);
}

void TouchPadService::SetVertScroll(bool vertScroll)
{
    m_vertScroll = vertScroll;
    Q_EMIT this->VertScrollChanged(vertScroll);
}

void TouchPadService::SetMotionAcceleration(double acceleration)
{
    m_motionAcceleration = acceleration;
    Q_EMIT this->MotionAccelerationChanged(acceleration);
}

void TouchPadService::SetDeltaScroll(qint32 deltaScroll)
{
    m_deltaScroll = deltaScroll;
    Q_EMIT this->DeltaScrollChanged(deltaScroll);
}

void TouchPadService::SetDoubleClick(qint32 interval)
{
    m_doubleClick = interval;
    Q_EMIT this->DoubleClickChanged(interval);
}

void TouchPadService::SetDragThreshold(qint32 threshold)
{
    m_dragThreshold = threshold;
    Q_EMIT this->DragThresholdChanged(threshold);
}

void TouchPadService::Reset()
{
    m_disableIfTyping = DisableIfTypingDefault;
    m_edgeScroll = EdgeScrollDefault;
    m_exist = ExistDefault;
    m_leftHanded = LeftHandedDefault;
    m_naturalScroll = NaturalScrollDefault;
    m_tPadEnable = TPadEnableDefault;
    m_tapClick = TapClickDefault;
    m_vertScroll = VertScrollDefault;
    m_motionAcceleration = MotionAccelerationDefault;
    m_deltaScroll = DeltaScrollDefault;
    m_doubleClick = DoubleClickDefault;
    m_dragThreshold = DragThresholdDefault;
    m_deviceList = DeviceListDefault;
}

DDEVICE_END_NAMESPACE
