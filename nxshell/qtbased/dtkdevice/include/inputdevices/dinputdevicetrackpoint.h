// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICETRACKPOINT_H
#define DINPUTDEVICETRACKPOINT_H
#include "dinputdevicepointer.h"
#include "dtkdevice_global.h"
#include <DExpected>
#include <QObject>
DDEVICE_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
class DInputDeviceTrackPointPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceTrackPoint : public DInputDevicePointer
{
    Q_OBJECT
    Q_PROPERTY(bool middleButtonEnabled READ middleButtonEnabled WRITE setMiddleButtonEnabled NOTIFY middleButtonEnabledChanged)
    Q_PROPERTY(qint32 middleButtonTimeout READ middleButtonTimeout WRITE setMiddleButtonTimeout NOTIFY middleButtonTimeoutChanged)
    Q_PROPERTY(bool wheelEmulation READ wheelEmulation WRITE setWheelEmulation NOTIFY wheelEmulationChanged)
    Q_PROPERTY(
        qint32 wheelEmulationButton READ wheelEmulationButton WRITE setWheelEmulationButton NOTIFY wheelEmulationButtonChanged)
    Q_PROPERTY(qint32 wheelEmulationTimeout READ wheelEmulationTimeout WRITE setWheelEmulationTimeout NOTIFY
                   wheelEmulationTimeoutChanged)
    Q_PROPERTY(
        bool wheelHorizontalScroll READ wheelHorizontalScroll WRITE setWheelHorizontalScroll NOTIFY wheelHorizontalScrollChanged)

public:
    ~DInputDeviceTrackPoint() override;

    bool middleButtonEnabled() const;
    qint32 middleButtonTimeout() const;
    bool wheelEmulation() const;
    qint32 wheelEmulationButton() const;
    qint32 wheelEmulationTimeout() const;
    bool wheelHorizontalScroll() const;

    void setMiddleButtonEnabled(bool enabled);
    void setMiddleButtonTimeout(qint32 timeout);
    void setWheelEmulation(bool emulation);
    void setWheelEmulationButton(qint32 button);
    void setWheelEmulationTimeout(qint32 timeout);
    void setWheelHorizontalScroll(bool horizontalScroll);

    bool leftHanded() const override;
    ScrollMethod scrollMethod() const override;
    AccelerationProfile accelerationProfile() const override;
    double accelerationSpeed() const override;
    void setLeftHanded(bool leftHanded) override;
    void setScrollMethod(ScrollMethod scrollMethod) override;
    void setAccelerationProfile(AccelerationProfile accelerationProfile) override;
    void setAccelerationSpeed(double accelerationSpeed) override;

Q_SIGNALS:

    void middleButtonEnabledChanged(bool enabled);
    void middleButtonTimeoutChanged(qint32 timeout);
    void wheelEmulationChanged(bool emulation);
    void wheelEmulationButtonChanged(qint32 button);
    void wheelEmulationTimeoutChanged(qint32 timeout);
    void wheelHorizontalScrollChanged(bool horizontalScroll);

public Q_SLOTS:
    DExpected<void> reset() override;

protected:
    DInputDeviceTrackPoint(const DeviceInfo &info = DeviceInfoInitializer, bool enabled = true, QObject *parent = nullptr);

private:
    friend class DInputDeviceManager;
    D_DECLARE_PRIVATE(DInputDeviceTrackPoint)
};
DDEVICE_END_NAMESPACE
#endif  // DINPUTDEVICETRACKPOINT_H
