// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDebug>
#include <QMetaType>

#include "dinputdevicesetting.h"
#include "dinputdevicesetting_p.h"
#include "dtkinputdevices_converters.h"
#include "inputdevicesinterface.h"
#include "keyboardinterface.h"
#include "mouseinterface.h"
#include "wacominterface.h"

DDEVICE_BEGIN_NAMESPACE
DInputDeviceSettingPrivate::DInputDeviceSettingPrivate(DInputDeviceSetting *q)
    : DTK_CORE_NAMESPACE::DObjectPrivate(q)
{
#if defined(USE_FAKE_INTERFACE)
    const QString &Service = QStringLiteral("org.deepin.dtk.InputDevices");
#else
    const QString &Service = QStringLiteral("com.deepin.daemon.InputDevices");
#endif
    m_inter = new InputDevicesInterface(Service);
    m_keyboardInter = new KeyboardInterface(Service);
    m_mouseInter = new MouseInterface(Service);
    m_wacomInter = new WacomInterface(Service);
}

DInputDeviceSettingPrivate::~DInputDeviceSettingPrivate()
{
    delete m_inter;
    delete m_keyboardInter;
    delete m_mouseInter;
    delete m_wacomInter;
}

DInputDeviceSetting::DInputDeviceSetting(QObject *parent)
    : QObject(parent)
    , DObject(*new DInputDeviceSettingPrivate(this))
{
    D_D(DInputDeviceSetting);
    qRegisterMetaType<KeyAction>("KeyAction");
    qRegisterMetaType<Key>("Key");
    connect(d->m_mouseInter, &MouseInterface::DisableTpadChanged, this, &DInputDeviceSetting::disableTouchPadWhileMouseChanged);
    connect(d->m_mouseInter, &MouseInterface::DoubleClickChanged, this, &DInputDeviceSetting::doubleClickIntervalChanged);
    connect(d->m_mouseInter, &MouseInterface::DragThresholdChanged, this, &DInputDeviceSetting::dragThresholdChanged);
    connect(d->m_keyboardInter, &KeyboardInterface::RepeatEnabledChanged, this, &DInputDeviceSetting::repeatEnabledChanged);
    connect(d->m_keyboardInter, &KeyboardInterface::RepeatDelayChanged, this, &DInputDeviceSetting::repeatDelayChanged);
    connect(d->m_keyboardInter, &KeyboardInterface::RepeatIntervalChanged, this, &DInputDeviceSetting::repeatIntervalChanged);
    connect(d->m_keyboardInter, &KeyboardInterface::CursorBlinkChanged, this, &DInputDeviceSetting::cursorBlinkIntervalChanged);
    connect(d->m_inter, &InputDevicesInterface::WheelSpeedChanged, this, &DInputDeviceSetting::wheelSpeedChanged);
}

DInputDeviceSetting::~DInputDeviceSetting() = default;

bool DInputDeviceSetting::disableTouchPadWhileMouse() const
{
    D_DC(DInputDeviceSetting);
    return d->m_mouseInter->DisableTpad();
}

quint32 DInputDeviceSetting::doubleClickInterval() const
{
    D_DC(DInputDeviceSetting);
    return d->m_mouseInter->DoubleClick();
}

quint32 DInputDeviceSetting::dragThreshold() const
{
    D_DC(DInputDeviceSetting);
    return d->m_mouseInter->DragThreshold();
}

bool DInputDeviceSetting::repeatEnabled() const
{
    D_DC(DInputDeviceSetting);
    return d->m_keyboardInter->RepeatEnabled();
}

quint32 DInputDeviceSetting::repeatDelay() const
{
    D_DC(DInputDeviceSetting);
    return d->m_keyboardInter->RepeatDelay();
}

quint32 DInputDeviceSetting::repeatInterval() const
{
    D_DC(DInputDeviceSetting);
    return d->m_keyboardInter->RepeatInterval();
}

qint32 DInputDeviceSetting::cursorBlinkInterval() const
{
    D_DC(DInputDeviceSetting);
    return d->m_keyboardInter->CursorBlink();
}

quint32 DInputDeviceSetting::wheelSpeed() const
{
    D_DC(DInputDeviceSetting);
    return d->m_inter->WheelSpeed();
}

void DInputDeviceSetting::setDisableTouchPadWhileMouse(bool disabled)
{
    D_D(DInputDeviceSetting);
    d->m_mouseInter->SetDisableTpad(disabled);
}

void DInputDeviceSetting::setDoubleClickInterval(quint32 interval)
{
    D_D(DInputDeviceSetting);
    d->m_mouseInter->SetDoubleClick(interval);
}

void DInputDeviceSetting::setDragThreshold(quint32 distance)
{
    D_D(DInputDeviceSetting);
    d->m_mouseInter->SetDragThreshold(distance);
}

void DInputDeviceSetting::setRepeatEnabled(bool enabled)
{
    D_D(DInputDeviceSetting);
    d->m_keyboardInter->SetRepeatEnabled(enabled);
}

void DInputDeviceSetting::setRepeatDelay(quint32 delay)
{
    D_D(DInputDeviceSetting);
    d->m_keyboardInter->SetRepeatDelay(delay);
}

void DInputDeviceSetting::setRepeatInterval(quint32 interval)
{
    D_D(DInputDeviceSetting);
    d->m_keyboardInter->SetRepeatInterval(interval);
}

void DInputDeviceSetting::setCursorBlinkInterval(qint32 interval)
{
    D_D(DInputDeviceSetting);
    d->m_keyboardInter->SetCursorBlink(interval);
}

void DInputDeviceSetting::setWheelSpeed(quint32 speed)
{
    D_D(DInputDeviceSetting);
    d->m_inter->SetWheelSpeed(speed);
}

DExpected<quint32> DInputDeviceSetting::pressureSensitivity(quint32 id)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    return d->m_wacomInter->EraserPressureSensitive();
}

DExpected<quint32> DInputDeviceSetting::rawSampleSize(quint32 id)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    return d->m_wacomInter->EraserRawSample();
}

DExpected<quint32> DInputDeviceSetting::pressureThreshold(quint32 id)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    return d->m_wacomInter->EraserThreshold();
}

DExpected<bool> DInputDeviceSetting::forceProportions(quint32 id)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    return d->m_wacomInter->ForceProportions();
}

DExpected<QString> DInputDeviceSetting::mapOutput(quint32 id)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    return d->m_wacomInter->MapOutput();
}

DExpected<bool> DInputDeviceSetting::mouseEnterRemap(quint32 id)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    return d->m_wacomInter->MouseEnterRemap();
}

DExpected<quint32> DInputDeviceSetting::suppress(quint32 id)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    return d->m_wacomInter->Suppress();
}

DExpected<void> DInputDeviceSetting::setPressureSensitivity(quint32 id, quint32 sensitivity)
{
    Q_UNUSED(id)
    D_D(DInputDeviceSetting);
    d->m_wacomInter->SetEraserPressureSensitive(sensitivity);
    return {};
}

DExpected<void> DInputDeviceSetting::setRawSampleSize(quint32 id, quint32 size)
{
    Q_UNUSED(id)
    D_D(DInputDeviceSetting);
    d->m_wacomInter->SetEraserRawSample(size);
    return {};
}

DExpected<void> DInputDeviceSetting::setPressureThreshold(quint32 id, quint32 threshold)
{
    Q_UNUSED(id)
    D_D(DInputDeviceSetting);
    d->m_wacomInter->SetEraserThreshold(threshold);
    return {};
}

DExpected<void> DInputDeviceSetting::setForceProportions(quint32 id, bool force)
{
    Q_UNUSED(id)
    D_D(DInputDeviceSetting);
    d->m_wacomInter->SetForceProportions(force);
    return {};
}

DExpected<void> DInputDeviceSetting::setMouseEnterRemap(quint32 id, bool remap)
{
    Q_UNUSED(id)
    D_D(DInputDeviceSetting);
    d->m_wacomInter->SetMouseEnterRemap(remap);
    return {};
}

DExpected<void> DInputDeviceSetting::setSuppress(quint32 id, quint32 suppress)
{
    Q_UNUSED(id)
    D_D(DInputDeviceSetting);
    d->m_wacomInter->SetSuppress(suppress);
    return {};
}

DExpected<KeyAction> DInputDeviceSetting::keymap(quint32 id, Key key)
{
    Q_UNUSED(id)
    D_DC(DInputDeviceSetting);
    QString ret;
    if (key == Key::KeyUp) {
        ret = d->m_wacomInter->KeyUpAction();
    } else {
        ret = d->m_wacomInter->KeyDownAction();
    }
    return stringToKeyAction(ret);
}

DExpected<void> DInputDeviceSetting::setKeymap(quint32 id, Key key, KeyAction action)
{
    Q_UNUSED(id)
    D_D(DInputDeviceSetting);
    if (key == Key::KeyUp) {
        d->m_wacomInter->SetKeyUpAction(keyActionToString(action));
    } else {
        d->m_wacomInter->SetKeyDownAction(keyActionToString(action));
    }
    return {};
}

DDEVICE_END_NAMESPACE
