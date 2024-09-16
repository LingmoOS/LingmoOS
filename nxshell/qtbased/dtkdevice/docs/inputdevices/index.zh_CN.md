@page inputdevices inputdevices
@brief dtkinputdevices模块

## 项目介绍

dtkinputdevices是对deepin和uos上输入设备的封装，输入设备包括键盘、鼠标、触摸板、数位板、指针杆等。除此之外，dtkinputdevices还提供全局的输入相关设置的访问与修改。

## 项目结构

该模块一共提供九个类：

+ **DInputDeviceManager** InputDevices管理类，可直接构造
+ **DInputDeviceSetting** 全局设置，可通过管理类获取
+ **DInputDevice** 设备抽象基类（通用设备类），可实例化，但是只能通过管理类实例化
+ **DInputDevicePointer** 抽象指针设备类，继承自DInputDevice，DInputDevicePointer为中间抽象层，不可实例化
+ **DInputDeviceMouse** 鼠标设备类，继承自DInputDevicePointer，可通过管理类实例化
+ **DInputDeviceTouchPad** 触摸板设备类，继承自DInputDevicePointer，可通过管理类实例化
+ **DInputDeviceTrackPoint** 指针杆设备类，继承自DInputDevicePointer，可通过管理类实例化
+ **DInputDeviceTablet** 数位板设备类，继承自DInputDevicePointer，可通过管理类实例化
+ **DInputDeviceKeyboard** 键盘设备类，继承自DInputDevice，可通过管理类实例化

## 使用实例

以下是一个简单的例子：

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

DDEVICE_USE_NAMESPACE // 该宏提供简单的使用命名空间的方式
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DInputDeviceManager manager; // 创建manager
    qDebug() << manager.deviceInfos();
    QObject::connect(&manager, &DInputDeviceManager::deviceAdded, &app, [&](const DeviceInfo &info) {
        qDebug() << "Device" << info << "removed.";
        qInfo() << "Devices:" << manager.deviceInfos();
    });
    QObject::connect(&manager, &DInputDeviceManager::deviceRemoved, &app, [&](const DeviceInfo &info) {
        qDebug() << "Device" << info << "removed.";
        qInfo() << "Devices:" << manager.deviceInfos();
    }); // 监听设备插拔信号
    auto infos = manager.deviceInfos(); // 获取所有设备信息
    foreach (const auto &info, infos) {
        auto expected = manager.createDevice(info); // 使用设备信息创建设备
        if (!expected) {
            qWarning() << "Can't create device using info" << info << ". Error code:" << expected.error().getErrorCode() << ","
                       << expected.error().getErrorMessage();
        } else {
            auto device = expected.value();
            qInfo() << "Device id:" << device->id() << ", name:" << device->name() << ", type:" << device->type()
                    << ", enabled:" << device->enabled();
            switch (info.type) {
                // 根据不同的设备类型进行相应的转换
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
@brief dtk输入设备模块
@details 示例文档:
@subpage inputdevices
@anchor group_inputdevices
