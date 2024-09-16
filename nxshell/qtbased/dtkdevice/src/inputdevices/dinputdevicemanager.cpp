// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dinputdevicemanager.h"
#include "dinputdevicemanager_p.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "dinputdevicekeyboard.h"
#include "dinputdevicemouse.h"
#include "dinputdevicesetting.h"
#include "dinputdevicetablet.h"
#include "dinputdevicetouchpad.h"
#include "dinputdevicetrackpoint.h"

DDEVICE_BEGIN_NAMESPACE
DInputDeviceManagerPrivate::DInputDeviceManagerPrivate(DInputDeviceManager *q)
    : DObjectPrivate(q)
{
#ifdef USE_FAKE_INTERFACE
    const QString &Service = QStringLiteral("org.deepin.dtk.InputDevices");
#else
    const QString &Service = QStringLiteral("com.deepin.daemon.InputDevices");
#endif
    m_mouseInter = new MouseInterface(Service);
    m_touchPadInter = new TouchPadInterface(Service);
    m_trackPointInter = new TrackPointInterface(Service);
    m_wacomInter = new WacomInterface(Service);
}

DInputDeviceManagerPrivate::~DInputDeviceManagerPrivate()
{
    delete m_mouseInter;
    delete m_touchPadInter;
    delete m_trackPointInter;
    delete m_wacomInter;
}

void DInputDeviceManagerPrivate::initializeDeviceInfos()
{
    DeviceInfo info;
    if (m_mouseInter->Exist()) {
        info.type = DeviceType::Mouse;
        QString deviceList = m_mouseInter->DeviceList();
        QJsonParseError parseErr;
        auto rootDoc = QJsonDocument::fromJson(deviceList.toLocal8Bit(), &parseErr);
        if (parseErr.error != QJsonParseError::NoError) {
            qWarning() << "Can't parse deviceList! Error:" << parseErr.error << "," << parseErr.errorString();
        } else {
            auto array = rootDoc.array();
            foreach (const auto &device, array) {
                info.id = device.toObject().value("Id").toInt();
                info.name = device.toObject().value("Name").toString();
                addDevice(info);
            }
        }
    }
    if (m_touchPadInter->Exist()) {
        info.type = DeviceType::TouchPad;
        QString deviceList = m_touchPadInter->DeviceList();
        QJsonParseError parseErr;
        auto rootDoc = QJsonDocument::fromJson(deviceList.toLocal8Bit(), &parseErr);
        if (parseErr.error != QJsonParseError::NoError) {
            qWarning() << "Can't parse deviceList! Error:" << parseErr.error << "," << parseErr.errorString();
        } else {
            auto array = rootDoc.array();
            foreach (const auto &device, array) {
                info.id = device.toObject().value("Id").toInt();
                info.name = device.toObject().value("Name").toString();
                addDevice(info);
            }
        }
    }
    if (m_trackPointInter->Exist()) {
        info.type = DeviceType::TrackPoint;
        QString deviceList = m_trackPointInter->DeviceList();
        QJsonParseError parseErr;
        auto rootDoc = QJsonDocument::fromJson(deviceList.toLocal8Bit(), &parseErr);
        if (parseErr.error != QJsonParseError::NoError) {
            qWarning() << "Can't parse deviceList! Error:" << parseErr.error << "," << parseErr.errorString();
        } else {
            auto array = rootDoc.array();
            foreach (const auto &device, array) {
                info.id = device.toObject().value("Id").toInt();
                info.name = device.toObject().value("Name").toString();
                addDevice(info);
            }
        }
    }
    if (m_wacomInter->Exist()) {
        info.type = DeviceType::Tablet;
        QString deviceList = m_wacomInter->DeviceList();
        QJsonParseError parseErr;
        auto rootDoc = QJsonDocument::fromJson(deviceList.toLocal8Bit(), &parseErr);
        if (parseErr.error != QJsonParseError::NoError) {
            qWarning() << "Can't parse deviceList! Error:" << parseErr.error << "," << parseErr.errorString();
        } else {
            auto array = rootDoc.array();
            foreach (const auto &device, array) {
                info.id = device.toObject().value("Id").toInt();
                info.name = device.toObject().value("Name").toString();
                addDevice(info);
            }
        }
    }
}

template <DeviceType deviceType> void DInputDeviceManager::handleDeviceChanged(const QString &deviceList)
{
    D_D(DInputDeviceManager);
    auto devices = d->m_idMap[deviceType];
    DeviceInfo info;
    info.type = deviceType;
    QJsonParseError parseErr;
    auto rootDoc = QJsonDocument::fromJson(deviceList.toLocal8Bit(), &parseErr);
    if (parseErr.error != QJsonParseError::NoError) {
        qWarning() << "Can't parse deviceList! Error:" << parseErr.error << "," << parseErr.errorString();
    } else {
        auto array = rootDoc.array();
        foreach (const auto &device, array) {
            info.id = device.toObject().value("Id").toInt();
            info.name = device.toObject().value("Name").toString();
            if (devices.contains(info.id)) {
                devices.removeAll(info.id);
            } else {
                d->addDevice(info);
            }
        }
    }
    foreach (const auto device, devices) {
        d->removeDeviceById(device);
    }
}

void DInputDeviceManagerPrivate::addDevice(const DeviceInfo &info)
{
    D_Q(DInputDeviceManager);
    if (!m_idMap[info.type].contains(info.id)) {
        m_deviceInfos.append(info);
        m_idMap[info.type].append(info.id);
        Q_EMIT q->deviceAdded(info);
    }
}

void DInputDeviceManagerPrivate::removeDevice(const DeviceInfo &info)
{
    D_Q(DInputDeviceManager);
    if (m_idMap[info.type].contains(info.id)) {
        m_deviceInfos.removeAll(info);
        m_idMap[info.type].removeAll(info.id);
        Q_EMIT q->deviceRemoved(info);
    }
}

void DInputDeviceManagerPrivate::removeDeviceById(quint32 id)
{
    foreach (const auto deviceInfo, m_deviceInfos) {
        if (deviceInfo.id == id) {
            removeDevice(deviceInfo);
            return;
        }
    }
}

DInputDeviceManager::DInputDeviceManager(QObject *parent)
    : QObject(parent)
    , DTK_CORE_NAMESPACE::DObject(*new DInputDeviceManagerPrivate(this))
{
    D_D(DInputDeviceManager);
    connect(
        d->m_mouseInter, &MouseInterface::DeviceListChanged, this, &DInputDeviceManager::handleDeviceChanged<DeviceType::Mouse>);
    connect(d->m_touchPadInter,
            &TouchPadInterface::DeviceListChanged,
            this,
            &DInputDeviceManager::handleDeviceChanged<DeviceType::TouchPad>);
    connect(d->m_trackPointInter,
            &TrackPointInterface::DeviceListChanged,
            this,
            &DInputDeviceManager::handleDeviceChanged<DeviceType::TrackPoint>);
    connect(
        d->m_wacomInter, &WacomInterface::DeviceListChanged, this, &DInputDeviceManager::handleDeviceChanged<DeviceType::Tablet>);
    d->initializeDeviceInfos();
}

DInputDeviceManager::~DInputDeviceManager() = default;

QList<DeviceInfo> DInputDeviceManager::deviceInfos() const
{
    D_DC(DInputDeviceManager);
    return d->m_deviceInfos;
}

DExpected<DInputDevicePtr> DInputDeviceManager::createDevice(const DeviceInfo &info)
{
    switch (info.type) {
        case DeviceType::Mouse:
            return DInputDevicePtr{new DInputDeviceMouse(info)};
        case DeviceType::TouchPad:
            return DInputDevicePtr{new DInputDeviceTouchPad(info)};
        case DeviceType::Tablet:
            return DInputDevicePtr{new DInputDeviceTablet(info)};
        case DeviceType::TrackPoint:
            return DInputDevicePtr{new DInputDeviceTrackPoint(info)};
        case DeviceType::Keyboard:
            return DInputDevicePtr{new DInputDeviceKeyboard(info)};
        case DeviceType::Generic:
            return DInputDevicePtr{new DInputDeviceGeneric(info)};
        default:
            return {};
    }
}
DExpected<DInputSettingPtr> DInputDeviceManager::setting()
{
    return DInputSettingPtr{new DInputDeviceSetting};
}
DDEVICE_END_NAMESPACE
