// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DDEVICEETTING_H
#define DDEVICEETTING_H

#include "dtkdevice_global.h"
#include "dtkinputdevices_types.h"

#include <DExpected>
#include <DObject>

#include <QObject>
#include <QSharedPointer>

using DInputKeyAction = DTK_DEVICE_NAMESPACE::KeyAction;
using DInputKey = DTK_DEVICE_NAMESPACE::Key;
DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
using DCORE_NAMESPACE::DObject;

class DInputDeviceSettingPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceSetting : public QObject, public DObject
{
    Q_OBJECT
    Q_PROPERTY(bool disableTouchPadWhileMouse READ disableTouchPadWhileMouse WRITE setDisableTouchPadWhileMouse NOTIFY
                   disableTouchPadWhileMouseChanged)
    Q_PROPERTY(
        quint32 doubleClickInterval READ doubleClickInterval WRITE setDoubleClickInterval NOTIFY doubleClickIntervalChanged)
    Q_PROPERTY(quint32 dragThreshold READ dragThreshold WRITE setDragThreshold NOTIFY dragThresholdChanged)
    Q_PROPERTY(bool repeatEnabled READ repeatEnabled WRITE setRepeatEnabled NOTIFY repeatEnabledChanged)
    Q_PROPERTY(quint32 repeatDelay READ repeatDelay WRITE setRepeatDelay NOTIFY repeatDelayChanged)
    Q_PROPERTY(quint32 repeatInterval READ repeatInterval WRITE setRepeatInterval NOTIFY repeatIntervalChanged)
    Q_PROPERTY(qint32 cursorBlinkInterval READ cursorBlinkInterval WRITE setCursorBlinkInterval NOTIFY cursorBlinkIntervalChanged)
    Q_PROPERTY(quint32 wheelSpeed READ wheelSpeed WRITE setWheelSpeed NOTIFY wheelSpeedChanged)

public:
    using Ptr = QSharedPointer<DInputDeviceSetting>;
    ~DInputDeviceSetting() override;

    bool disableTouchPadWhileMouse() const;
    quint32 doubleClickInterval() const;
    quint32 dragThreshold() const;
    bool repeatEnabled() const;
    quint32 repeatDelay() const;
    quint32 repeatInterval() const;
    qint32 cursorBlinkInterval() const;
    quint32 wheelSpeed() const;

    void setDisableTouchPadWhileMouse(bool disabled);
    void setDoubleClickInterval(quint32 interval);
    void setDragThreshold(quint32 distance);
    void setRepeatEnabled(bool enabled);
    void setRepeatDelay(quint32 delay);
    void setRepeatInterval(quint32 interval);
    void setCursorBlinkInterval(qint32 interval);
    void setWheelSpeed(quint32 speed);

Q_SIGNALS:
    void disableTouchPadWhileMouseChanged(bool disabled);
    void doubleClickIntervalChanged(quint32 interval);
    void dragThresholdChanged(quint32 distance);
    void repeatEnabledChanged(bool enabled);
    void repeatDelayChanged(quint32 delay);
    void repeatIntervalChanged(quint32 interval);
    void cursorBlinkIntervalChanged(qint32 interval);
    void wheelSpeedChanged(quint32 speed);

public Q_SLOTS:
    DExpected<quint32> pressureSensitivity(quint32 id);
    DExpected<quint32> rawSampleSize(quint32 id);
    DExpected<quint32> pressureThreshold(quint32 id);
    DExpected<bool> forceProportions(quint32 id);
    DExpected<QString> mapOutput(quint32 id);
    DExpected<bool> mouseEnterRemap(quint32 id);
    DExpected<quint32> suppress(quint32 id);
    DExpected<void> setPressureSensitivity(quint32 id, quint32 sensitivity);
    DExpected<void> setRawSampleSize(quint32 id, quint32 size);
    DExpected<void> setPressureThreshold(quint32 id, quint32 threshold);
    DExpected<void> setForceProportions(quint32 id, bool force);
    DExpected<void> setMouseEnterRemap(quint32 id, bool remap);
    DExpected<void> setSuppress(quint32 id, quint32 suppress);
    DExpected<DInputKeyAction> keymap(quint32 id, DInputKey key);
    DExpected<void> setKeymap(quint32 id, DInputKey key, DInputKeyAction action);

private:
    explicit DInputDeviceSetting(QObject *parent = nullptr);
    friend class DInputDeviceManager;
    D_DECLARE_PRIVATE(DInputDeviceSetting)
};
DDEVICE_END_NAMESPACE

#endif
