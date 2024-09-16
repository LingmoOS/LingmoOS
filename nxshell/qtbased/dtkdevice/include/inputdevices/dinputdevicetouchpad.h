// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DINPUTDEVICETOUCHPAD_H
#define DINPUTDEVICETOUCHPAD_H

#include "dtkdevice_global.h"

#include <DExpected>
#include <QObject>

#include "dinputdevicepointer.h"

DDEVICE_BEGIN_NAMESPACE

using DCORE_NAMESPACE::DExpected;
class DInputDeviceTouchPadPrivate;
class LIBDTKDEVICESHARED_EXPORT DInputDeviceTouchPad : public DInputDevicePointer
{
    Q_OBJECT
    Q_PROPERTY(bool disableWhileTyping READ disableWhileTyping WRITE setDisableWhileTyping NOTIFY disableWhileTypingChanged)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)
    Q_PROPERTY(bool tapToClick READ tapToClick WRITE setTapToClick NOTIFY tapToClickChanged)

public:
    ~DInputDeviceTouchPad() override;

    bool disableWhileTyping() const;
    bool naturalScroll() const;
    bool tapToClick() const;

    void setDisableWhileTyping(bool disableWhileTyping);
    void setNaturalScroll(bool naturalScroll);
    void setTapToClick(bool tapToClick);

    bool enabled() const override;
    bool leftHanded() const override;
    ScrollMethod scrollMethod() const override;
    AccelerationProfile accelerationProfile() const override;
    double accelerationSpeed() const override;
    void setLeftHanded(bool leftHanded) override;
    void setScrollMethod(ScrollMethod scrollMethod) override;
    void setAccelerationProfile(AccelerationProfile accelerationProfile) override;
    void setAccelerationSpeed(double accelerationSpeed) override;

public Q_SLOTS:
    DExpected<void> reset() override;
    DExpected<void> enable(bool enabled = true);

Q_SIGNALS:
    void disableWhileTypingChanged(bool disableWhileTyping);
    void naturalScrollChanged(bool naturalScroll);
    void tapToClickChanged(bool tapToClick);

protected:
    explicit DInputDeviceTouchPad(const DeviceInfo &info = DeviceInfoInitializer, bool enabled = true, QObject *parent = nullptr);

private:
    friend class DInputDeviceManager;
    D_DECLARE_PRIVATE(DInputDeviceTouchPad)
};

DDEVICE_END_NAMESPACE

#endif  // DINPUTDEVICETOUCHPAD_H
