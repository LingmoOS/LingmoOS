/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "winbattery.h"
#include "windevicemanager_p.h"

#include <batclass.h>
#include <devguid.h>
#include <devpropdef.h>
#include <setupapi.h>

using namespace Solid::Backends::Win;

QMap<QString, WinBattery::Battery> WinBattery::m_udiToGDI = QMap<QString, WinBattery::Battery>();

WinBattery::WinBattery(WinDevice *device)
    : WinInterface(device)
    , m_state(Solid::Battery::NoCharge)
{
    powerChanged();
    connect(SolidWinEventFilter::instance(), SIGNAL(powerChanged()), this, SLOT(powerChanged()));
}

Solid::Battery::BatteryType WinBattery::type() const
{
    return m_type;
}

int WinBattery::chargePercent() const
{
    return m_charge;
}

int WinBattery::capacity() const
{
    return m_capacity;
}

bool WinBattery::isRechargeable() const
{
    return m_rechargeable;
}

bool WinBattery::isPowerSupply() const
{
    return m_isPowerSupply;
}

Solid::Battery::ChargeState WinBattery::chargeState() const
{
    return m_state;
}

QSet<QString> WinBattery::getUdis()
{
    QSet<QString> udis;
    HDEVINFO hdev = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (INVALID_HANDLE_VALUE != hdev) {
        // Limit search to 100 batteries max
        for (int idev = 0; idev < 100; idev++) {
            SP_DEVICE_INTERFACE_DATA did;
            ZeroMemory(&did, sizeof(did));
            did.cbSize = sizeof(did);

            if (SetupDiEnumDeviceInterfaces(hdev, 0, &GUID_DEVCLASS_BATTERY, idev, &did)) {
                DWORD cbRequired = 0;

                SetupDiGetDeviceInterfaceDetailW(hdev, &did, 0, 0, &cbRequired, 0);
                if (ERROR_INSUFFICIENT_BUFFER == GetLastError()) {
                    char *buffer = new char[cbRequired];
                    SP_DEVICE_INTERFACE_DETAIL_DATA *pdidd = (SP_DEVICE_INTERFACE_DETAIL_DATA *)buffer;
                    ZeroMemory(pdidd, cbRequired);
                    pdidd->cbSize = sizeof(*pdidd);
                    if (SetupDiGetDeviceInterfaceDetail(hdev, &did, pdidd, cbRequired, &cbRequired, 0)) {
                        QString path = QString::fromWCharArray(pdidd->DevicePath);
                        ulong tag = WinDeviceManager::getDeviceInfo<ulong>(path, IOCTL_BATTERY_QUERY_TAG);
                        QString udi = QLatin1String("/org/kde/solid/win/power.battery/battery#") + QString::number(tag);
                        udis << udi;
                        m_udiToGDI[udi] = Battery(path, tag);
                    }
                    delete[] buffer;
                }
            }
        }

        SetupDiDestroyDeviceInfoList(hdev);
    }
    return udis;
}

const WinBattery::Battery WinBattery::batteryInfoFromUdi(const QString &udi)
{
    return m_udiToGDI[udi];
}

void WinBattery::powerChanged()
{
    const int old_charge = m_charge;
    const int old_capacity = m_capacity;
    const Solid::Battery::ChargeState old_state = m_state;
    const bool old_isPowerSupply = m_isPowerSupply;
    const double old_energy = m_energy;
    const double old_energyFull = m_energyFull;
    const double old_energyFullDesign = m_energyFullDesign;
    const double old_energyRate = m_energyRate;
    const double old_voltage = m_voltage;

    BATTERY_WAIT_STATUS batteryStatusQuery;
    ZeroMemory(&batteryStatusQuery, sizeof(batteryStatusQuery));
    Battery b = m_udiToGDI[m_device->udi()];
    batteryStatusQuery.BatteryTag = b.second;
    BATTERY_STATUS status = WinDeviceManager::getDeviceInfo<BATTERY_STATUS, BATTERY_WAIT_STATUS>(b.first, IOCTL_BATTERY_QUERY_STATUS, &batteryStatusQuery);

    BATTERY_QUERY_INFORMATION batteryInformationQuery;
    ZeroMemory(&batteryInformationQuery, sizeof(batteryInformationQuery));
    batteryInformationQuery.BatteryTag = b.second;
    batteryInformationQuery.InformationLevel = BatteryInformation;
    BATTERY_INFORMATION info =
        WinDeviceManager::getDeviceInfo<BATTERY_INFORMATION, BATTERY_QUERY_INFORMATION>(b.first, IOCTL_BATTERY_QUERY_INFORMATION, &batteryInformationQuery);

    initSerial(b);
    updateBatteryTemp(b);
    updateTimeToEmpty(b);

    m_isPowerSupply = !(status.PowerState & BATTERY_POWER_ON_LINE);

    QString tech = QString::fromUtf8((const char *)info.Chemistry, 4);

    if (tech == "LION" || tech == "LI-I") {
        m_technology = Solid::Battery::LithiumIon;
    } else if (tech == "PBAC") {
        m_technology = Solid::Battery::LeadAcid;
    } else if (tech == "NICD") {
        m_technology = Solid::Battery::NickelCadmium;
    } else if (tech == "NIMH") {
        m_technology = Solid::Battery::NickelMetalHydride;
    } else {
        m_technology = Solid::Battery::UnknownTechnology;
    }

    m_energy = status.Capacity / 1000.0; // provided in mWh
    m_energyFull = info.FullChargedCapacity / 1000.0; // provided in mWh
    m_energyFullDesign = info.DesignedCapacity / 1000.0; // provided in mWh
    m_energyRate = status.Rate / 1000.0; // provided in mW
    m_voltage = status.Voltage / 1000.0; // provided in mV

    if (info.FullChargedCapacity != 0) {
        m_charge = (float)status.Capacity / info.FullChargedCapacity * 100.0;
    }

    if (info.DesignedCapacity != 0) {
        m_capacity = (float)info.FullChargedCapacity / info.DesignedCapacity * 100.0;
    }

    if (status.PowerState == 0) {
        m_state = Solid::Battery::NoCharge;
    } else if (status.PowerState & BATTERY_CHARGING) {
        m_state = Solid::Battery::Charging;
    } else if (status.PowerState & BATTERY_DISCHARGING) {
        m_state = Solid::Battery::Discharging;
    }
    //    else if(info.PowerState & 0x00000008)//critical
    //    {

    //    }

    if (info.Capabilities & BATTERY_SYSTEM_BATTERY) {
        m_type = Solid::Battery::PrimaryBattery;
    } else {
        m_type = Solid::Battery::UnknownBattery;
    }

    m_rechargeable = info.Technology == 1;

    if (m_charge != old_charge) {
        Q_EMIT chargePercentChanged(m_charge, m_device->udi());
    }

    if (m_capacity != old_capacity) {
        Q_EMIT capacityChanged(m_capacity, m_device->udi());
    }

    if (old_state != m_state) {
        Q_EMIT chargeStateChanged(m_state, m_device->udi());
    }

    if (old_isPowerSupply != m_isPowerSupply) {
        Q_EMIT powerSupplyStateChanged(m_isPowerSupply, m_device->udi());
    }

    if (old_energy != m_energy) {
        Q_EMIT energyChanged(m_energy, m_device->udi());
    }

    if (old_energyFull != m_energyFull) {
        Q_EMIT energyFullChanged(m_energyFull, m_device->udi());
    }

    if (old_energyFullDesign != m_energyFullDesign) {
        Q_EMIT energyFullDesignChanged(m_energyFullDesign, m_device->udi());
    }

    if (old_energyRate != m_energyRate) {
        Q_EMIT energyRateChanged(m_energyRate, m_device->udi());
    }

    if (old_voltage != m_voltage) {
        Q_EMIT voltageChanged(m_voltage, m_device->udi());
    }
}

void WinBattery::initSerial(const Battery &b)
{
    wchar_t buffer[1024];
    BATTERY_QUERY_INFORMATION batteryInformationQuery;
    ZeroMemory(&batteryInformationQuery, sizeof(batteryInformationQuery));
    batteryInformationQuery.BatteryTag = b.second;
    batteryInformationQuery.InformationLevel = BatterySerialNumber;
    WinDeviceManager::getDeviceInfo<wchar_t, BATTERY_QUERY_INFORMATION>(b.first, IOCTL_BATTERY_QUERY_INFORMATION, buffer, 1024, &batteryInformationQuery);

    m_serial = QString::fromWCharArray(buffer);
}

void WinBattery::updateTimeToEmpty(const WinBattery::Battery &b)
{
    BATTERY_QUERY_INFORMATION batteryInformationQuery;
    ZeroMemory(&batteryInformationQuery, sizeof(batteryInformationQuery));
    batteryInformationQuery.BatteryTag = b.second;
    batteryInformationQuery.InformationLevel = BatteryEstimatedTime;
    ulong time = WinDeviceManager::getDeviceInfo<ulong, BATTERY_QUERY_INFORMATION>(b.first, IOCTL_BATTERY_QUERY_INFORMATION, &batteryInformationQuery);

    if (time == BATTERY_UNKNOWN_TIME) {
        time = 0;
    }

    if (time != m_timeUntilEmpty) {
        m_timeUntilEmpty = time;
        Q_EMIT timeToEmptyChanged(time, m_device->udi());
    }
}

void WinBattery::updateBatteryTemp(const WinBattery::Battery &b)
{
    BATTERY_QUERY_INFORMATION batteryInformationQuery;
    ZeroMemory(&batteryInformationQuery, sizeof(batteryInformationQuery));
    batteryInformationQuery.BatteryTag = b.second;
    batteryInformationQuery.InformationLevel = BatteryTemperature;
    ulong batteryTemp = WinDeviceManager::getDeviceInfo<ulong, BATTERY_QUERY_INFORMATION>(b.first, IOCTL_BATTERY_QUERY_INFORMATION, &batteryInformationQuery);

    if (batteryTemp != m_temperature) {
        m_temperature = batteryTemp;
        Q_EMIT temperatureChanged(batteryTemp, m_device->udi());
    }
}

Solid::Battery::Technology WinBattery::technology() const
{
    return m_technology;
}

double WinBattery::energy() const
{
    return m_energy;
}

double WinBattery::energyFull() const
{
    return m_energyFull;
}

double WinBattery::energyFullDesign() const
{
    return m_energyFullDesign;
}

double WinBattery::energyRate() const
{
    return m_energyRate;
}

double WinBattery::voltage() const
{
    return m_voltage;
}

bool WinBattery::isPresent() const
{
    return true;
}

qlonglong WinBattery::timeToEmpty() const
{
    return m_timeUntilEmpty;
}

qlonglong WinBattery::timeToFull() const
{
    return 0;
}

double WinBattery::temperature() const
{
    return m_temperature;
}

qlonglong WinBattery::remainingTime() const
{
    return m_timeUntilEmpty; // FIXME
}

QString WinBattery::serial() const
{
    return m_serial;
}

#include "moc_winbattery.cpp"
