// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicegeneric.h"
#include "dinputdevicemanager.h"
#include "dinputdevicemouse.h"
#include "dinputdevicesetting.h"
#include "dinputdevicetablet.h"
#include "dinputdevicetouchpad.h"
#include "dinputdevicetrackpoint.h"
#include <QCoreApplication>
#include <QObject>

DDEVICE_USE_NAMESPACE
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DInputDeviceManager manager;
    qDebug() << manager.deviceInfos();
    QObject::connect(&manager, &DInputDeviceManager::deviceAdded, &app, [&](const DeviceInfo &info) {
        qDebug() << "Device" << info << "added.";
        qInfo() << "Devices:" << manager.deviceInfos();
    });
    QObject::connect(&manager, &DInputDeviceManager::deviceRemoved, &app, [&](const DeviceInfo &info) {
        qDebug() << "Device" << info << "removed.";
        qInfo() << "Devices:" << manager.deviceInfos();
    });
    auto infos = manager.deviceInfos();
    foreach (const auto &info, infos) {
        auto expected = manager.createDevice(info);
        if (!expected) {
            qWarning() << "Can't create device using info" << info << ". Error code:" << expected.error().getErrorCode() << ","
                       << expected.error().getErrorMessage();
        } else {
            auto device = expected.value();
            qInfo() << "Device id:" << device->id() << ", name:" << device->name() << ", type:" << device->type()
                    << ", enabled:" << device->enabled();
            switch (info.type) {
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
