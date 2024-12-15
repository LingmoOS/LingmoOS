// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dapplicationeventmonitor.h"

#include <QGuiApplication>
#include <QInputEvent>

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#include <QPointingDevice>
typedef QPointingDevice QTouchDevice;
#else
#include <QTouchDevice>
#endif

DPP_BEGIN_NAMESPACE

DApplicationEventMonitor::DApplicationEventMonitor(QObject *parent)
    : QObject(parent)
    , m_lastInputDeviceType(None)
{
    qApp->installEventFilter(this);
}

DApplicationEventMonitor::~DApplicationEventMonitor()
{
}

DApplicationEventMonitor::InputDeviceType DApplicationEventMonitor::lastInputDeviceType() const
{
    return m_lastInputDeviceType;
}

DApplicationEventMonitor::InputDeviceType DApplicationEventMonitor::eventType(QEvent *event)
{
    Q_ASSERT(event);

    InputDeviceType last_input_device_type = None;

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *pMouseEvent = static_cast<QMouseEvent *>(event);

        if (pMouseEvent->source() == Qt::MouseEventNotSynthesized) { //由真实鼠标事件生成
            last_input_device_type = Mouse;
        }
        break;
    }
    case QEvent::TabletPress:
    case QEvent::TabletRelease:
    case QEvent::TabletMove:
        last_input_device_type = Tablet;
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        last_input_device_type = Keyboard;
        break;
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    case QEvent::TouchCancel: {
        QTouchEvent *pTouchEvent = static_cast<QTouchEvent *>(event);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        if (pTouchEvent->device()->type() == QInputDevice::DeviceType::TouchScreen) {
#else
        if (pTouchEvent->device()->type() == QTouchDevice::TouchScreen) {
#endif
            last_input_device_type = TouchScreen;
        }
        break;
    }
    default:
        break;
    }

    return last_input_device_type;
}

bool DApplicationEventMonitor::eventFilter(QObject *watched, QEvent *event)
{
    auto last_input_device_type = eventType(event);

    if (last_input_device_type != None && last_input_device_type != m_lastInputDeviceType) {
        m_lastInputDeviceType = last_input_device_type;
        Q_EMIT lastInputDeviceTypeChanged();
    }

    return QObject::eventFilter(watched, event);
}

DPP_END_NAMESPACE
