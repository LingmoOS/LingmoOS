/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "windevicemanager.h"
#include "winbattery.h"
#include "winblock.h"
#include "windevice.h"
#include "windevicemanager_p.h"
#include "winprocessor.h"
#include <solid/deviceinterface.h>

#include <dbt.h>

using namespace Solid::Backends::Win;

Q_GLOBAL_STATIC(SolidWinEventFilter, solidWineventFilter)

SolidWinEventFilter *SolidWinEventFilter::instance()
{
    return solidWineventFilter;
}

SolidWinEventFilter::SolidWinEventFilter()
    : QObject()
{
    wchar_t title[] = L"KDEWinDeviceManager";

    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.lpfnWndProc = SolidWinEventFilter::WndProc;
    wcex.hInstance = (HINSTANCE)::GetModuleHandle(NULL);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    wcex.lpszClassName = title;
    if (RegisterClassEx(&wcex) == 0) {
        qWarning() << "Failed to initialize KDEWinDeviceManager we will be unable to detect device changes";
        return;
    }

    m_windowID = CreateWindow(title, //
                              title,
                              WS_ICONIC,
                              0,
                              0,
                              CW_USEDEFAULT,
                              0,
                              NULL,
                              NULL,
                              wcex.hInstance,
                              NULL);
    if (m_windowID == NULL) {
        qWarning() << "Failed to initialize KDEWinDeviceManager we will be unable to detect device changes";
        return;
    }
    ShowWindow(m_windowID, SW_HIDE);
}

SolidWinEventFilter::~SolidWinEventFilter()
{
    PostMessage(m_windowID, WM_CLOSE, 0, 0);
}

void SolidWinEventFilter::promoteAddedDevice(const QSet<QString> &udi)
{
    Q_EMIT deviceAdded(udi);
}

void SolidWinEventFilter::promoteRemovedDevice(const QSet<QString> &udi)
{
    Q_EMIT deviceRemoved(udi);
}

void SolidWinEventFilter::promotePowerChange()
{
    Q_EMIT powerChanged();
}

LRESULT CALLBACK SolidWinEventFilter::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // some parts of the code are based on http://www.codeproject.com/Articles/119168/Hardware-Change-Detection
    switch (message) {
    case WM_DEVICECHANGE: {
        if ((wParam == DBT_DEVICEARRIVAL) || (wParam == DBT_DEVICEREMOVECOMPLETE)) {
            DEV_BROADCAST_HDR *header = reinterpret_cast<DEV_BROADCAST_HDR *>(lParam);
            if (header->dbch_devicetype == DBT_DEVTYP_VOLUME) {
                DEV_BROADCAST_VOLUME *devNot = reinterpret_cast<DEV_BROADCAST_VOLUME *>(lParam);
                switch (wParam) {
                case DBT_DEVICEREMOVECOMPLETE: {
                    QSet<QString> udis = WinBlock::getFromBitMask(devNot->dbcv_unitmask);
                    solidWineventFilter->promoteRemovedDevice(udis);
                    break;
                }
                case DBT_DEVICEARRIVAL: {
                    QSet<QString> udis = WinBlock::updateUdiFromBitMask(devNot->dbcv_unitmask);
                    solidWineventFilter->promoteAddedDevice(udis);
                    break;
                }
                }
                break;
            }
        }
        break;
    }
    case WM_POWERBROADCAST: {
        solidWineventFilter->promotePowerChange();
        break;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

WinDeviceManager::WinDeviceManager(QObject *parent)
    : DeviceManager(parent)
{
    connect(solidWineventFilter, SIGNAL(deviceAdded(QSet<QString>)), this, SLOT(slotDeviceAdded(QSet<QString>)));
    connect(solidWineventFilter, SIGNAL(deviceRemoved(QSet<QString>)), this, SLOT(slotDeviceRemoved(QSet<QString>)));

    // clang-format off
    m_supportedInterfaces << Solid::DeviceInterface::GenericInterface
                             //                          << Solid::DeviceInterface::Block
                          << Solid::DeviceInterface::StorageAccess
                          << Solid::DeviceInterface::StorageDrive
                          << Solid::DeviceInterface::OpticalDrive
                          << Solid::DeviceInterface::StorageVolume
                          << Solid::DeviceInterface::OpticalDisc
                          << Solid::DeviceInterface::Processor
                          << Solid::DeviceInterface::Battery;
    // clang-format on

    updateDeviceList();
}

WinDeviceManager::~WinDeviceManager()
{
}

QString WinDeviceManager::udiPrefix() const
{
    return QString();
}

QSet<Solid::DeviceInterface::Type> Solid::Backends::Win::WinDeviceManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList WinDeviceManager::allDevices()
{
    return m_devicesList;
}

QStringList WinDeviceManager::devicesFromQuery(const QString &parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList list;
    const QStringList deviceList = allDevices();
    if (!parentUdi.isEmpty()) {
        for (const QString &udi : deviceList) {
            WinDevice device(udi);
            if (device.type() == type && device.parentUdi() == parentUdi) {
                list << udi;
            }
        }
    } else if (type != Solid::DeviceInterface::Unknown) {
        for (const QString &udi : deviceList) {
            WinDevice device(udi);
            if (device.queryDeviceInterface(type)) {
                list << udi;
            }
        }
    } else {
        list << deviceList;
    }
    return list;
}

QObject *Solid::Backends::Win::WinDeviceManager::createDevice(const QString &udi)
{
    if (allDevices().contains(udi)) {
        return new WinDevice(udi);
    } else {
        return 0;
    }
}

void WinDeviceManager::slotDeviceAdded(const QSet<QString> &udi)
{
    const QSet<QString> tmp = udi - m_devices; // don't report devices that are already known(cd drive)
    m_devices += tmp;
    m_devicesList = QStringList(m_devices.begin(), m_devices.end());
    std::sort(m_devicesList.begin(), m_devicesList.end());
    for (const QString &str : tmp) {
        Q_EMIT deviceAdded(str);
    }
}

void WinDeviceManager::slotDeviceRemoved(const QSet<QString> &udi)
{
    m_devices -= udi;
    m_devicesList = QStringList(m_devices.begin(), m_devices.end());
    std::sort(m_devicesList.begin(), m_devicesList.end());
    for (const QString &str : udi) {
        Q_EMIT deviceRemoved(str);
    }
}

void WinDeviceManager::updateDeviceList()
{
    QSet<QString> devices = WinProcessor::getUdis();
    devices += WinBlock::getUdis();
    devices += WinBattery::getUdis();

    m_devices = devices;
    m_devicesList = QStringList(m_devices.begin(), m_devices.end());
    std::sort(m_devicesList.begin(), m_devicesList.end());
}

#include "moc_windevicemanager.cpp"
#include "moc_windevicemanager_p.cpp"
