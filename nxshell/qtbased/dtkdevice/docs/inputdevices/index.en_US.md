@page inputdevices inputdevices
@brief dtkinputdevices module

## Introduction

dtkinputdevices provides access to input devices and global input settings on deepin and uos.

## Structure

There are 9 classes in this module:

+ **DInputDeviceManager** InputDevices manager, can be directly constructed
+ **DInputDeviceSetting** global settings, can be acquired from manager
+ **DInputDevice** abstract device base class (also generic device class), can be instantiated by manager
+ **DInputDevicePointer** abstract pointer device, as a abstract layer, it cannot be instantiated
+ **DInputDeviceMouse** mouse device, derived from DInputDevicePointer, can be instantiated by manager
+ **DInputDeviceTouchPad** touch pad device, derived from DInputDevicePointer, can be instantiated by manager
+ **DInputDeviceTrackPoint** track point device, derived from DInputDevicePointer, can be instantiated by manager
+ **DInputDeviceTablet** tablet pad and tablet tools device, derived from DInputDevicePointer, can be instantiated by manager
+ **DInputDeviceKeyboard** keyboard device, derived from DInputDevice, can be instantiated by manager

## Example

Here is a simple example：

```cpp
#include "dinputdevice.h"
#include "dinputdevicemanager.h"
#include "dinputdevicemouse.h"
#include "dinputdevicesetting.h"
#include "dinputdevicetablet.h"
#include "dinputdevicetouchpad.h"
#include "dinputdevicetrackpoint.h"
#include <QCoreApplication>
#include <QObject>

DDEVICE_USE_NAMESPACE // use namespace
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DInputDeviceManager manager; // create a manager
    qDebug() << manager.deviceInfos();
    QObject::connect(&manager, &DInputDeviceManager::deviceAdded, &app, [&](const DeviceInfo &info) {
        qDebug() << "Device" << info << "removed.";
        qInfo() << "Devices:" << manager.deviceInfos();
    });
    QObject::connect(&manager, &DInputDeviceManager::deviceRemoved, &app, [&](const DeviceInfo &info) {
        qDebug() << "Device" << info << "removed.";
        qInfo() << "Devices:" << manager.deviceInfos();
    }); // monitor plug and unplug signals
    auto infos = manager.deviceInfos(); // get all device infos
    foreach (const auto &info, infos) {
        auto expected = manager.createDevice(info); // create device by info
        if (!expected) {
            qWarning() << "Can't create device using info" << info << ". Error code:" << expected.error().getErrorCode() << ","
                       << expected.error().getErrorMessage();
        } else {
            auto device = expected.value();
            qInfo() << "Device id:" << device->id() << ", name:" << device->name() << ", type:" << device->type()
                    << ", enabled:" << device->enabled();
            switch (info.type) {
                // cast to specific device by type
                case DeviceType::Mouse: {
                    auto mouse = device.dynamicCast<DInputDeviceMouse>();
                    qDebug() << "Acceleration profile:" << mouse->accelerationProfile();
                    break;
                }
                case DeviceType::TouchPad: {
                    auto touchPad = device.dynamicCast<DInputDeviceTouchPad>();
                    qDebug() << "Disable while typing:" << touchPad->disableWhileTyping();
                    qDebug() << "Acceleration speed:" << touchPad->accelerationSpeed();
                    break;
                }
                case DeviceType::TrackPoint: {
                    auto trackPoint = device.dynamicCast<DInputDeviceTrackPoint>();
                    qDebug() << "Middle button timeout:" << trackPoint->middleButtonTimeout();
                    break;
                }
                case DeviceType::Tablet: {
                    auto tablet = device.dynamicCast<DInputDeviceTablet>();
                    qDebug() << "Cursor mode:" << tablet->cursorMode();
                    break;
                }
                case DeviceType::Keyboard:
                case DeviceType::Generic:
                default:
                    qWarning() << "These devices is not implemented or recognized.";
                    break;
            }
        }
    }
    auto eSetting = manager.setting();
    if (!eSetting) {
        qWarning() << "Can't get default setting. Error code:" << eSetting.error().getErrorCode() << ","
                   << eSetting.error().getErrorMessage();
    } else {
        auto setting = eSetting.value();
        qDebug() << "Cursor blink interval:" << setting->cursorBlinkInterval();
        qDebug() << "Wheel speed:" << setting->wheelSpeed();
    }
    return app.exec();
}
```

@defgroup inputdevices
@brief dtkinputdevices module
@details example document:
@subpage inputdevices
@anchor group_inputdevices
