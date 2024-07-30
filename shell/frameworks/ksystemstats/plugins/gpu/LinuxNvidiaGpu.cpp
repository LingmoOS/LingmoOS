/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "LinuxNvidiaGpu.h"

#include <libudev.h>

NvidiaSmiProcess *LinuxNvidiaGpu::s_smiProcess = nullptr;

constexpr quint64 mbToBytes(quint64 mb) {
    return mb * 1024 * 1024;
}

LinuxNvidiaGpu::LinuxNvidiaGpu(const QString &id, const QString &name, udev_device *device)
    : GpuDevice(id, name)
    , m_device(device)
{
    if (!s_smiProcess) {
        s_smiProcess = new NvidiaSmiProcess();
    }

    connect(s_smiProcess, &NvidiaSmiProcess::dataReceived, this, &LinuxNvidiaGpu::onDataReceived);
}

LinuxNvidiaGpu::~LinuxNvidiaGpu()
{
    for (auto sensor : {m_usageProperty, m_totalVramProperty, m_usedVramProperty, m_temperatureProperty, m_coreFrequencyProperty, m_memoryFrequencyProperty}) {
        if (sensor->isSubscribed()) {
            LinuxNvidiaGpu::s_smiProcess->unref();
        }
    }
}

void LinuxNvidiaGpu::initialize()
{
    GpuDevice::initialize();

    for (auto sensor : {m_usageProperty,
                        m_totalVramProperty,
                        m_usedVramProperty,
                        m_temperatureProperty,
                        m_coreFrequencyProperty,
                        m_memoryFrequencyProperty,
                        m_powerProperty}) {
        connect(sensor, &KSysGuard::SensorProperty::subscribedChanged, sensor, [sensor]() {
            if (sensor->isSubscribed()) {
                LinuxNvidiaGpu::s_smiProcess->ref();
            } else {
                LinuxNvidiaGpu::s_smiProcess->unref();
            }
        });
    }

    const auto queryResult = s_smiProcess->query();
    const auto sysName = QString::fromLocal8Bit(udev_device_get_sysname(m_device));
    auto it = std::find_if(queryResult.cbegin(), queryResult.cend(), [&sysName] (const NvidiaSmiProcess::GpuQueryResult &result) {
        return result.pciPath == sysName;
    });
    if (it == queryResult.cend()) {
        qWarning() << "Could not retrieve information for NVidia GPU" << sysName;
    } else {
        m_index = it - queryResult.cbegin();
        m_nameProperty->setValue(it->name);
        m_totalVramProperty->setValue(mbToBytes(it->totalMemory));
        m_usedVramProperty->setMax(mbToBytes(it->totalMemory));
        m_coreFrequencyProperty->setMax(it->maxCoreFrequency);
        m_memoryFrequencyProperty->setMax(it->maxMemoryFrequency);
        m_temperatureProperty->setMax(it->maxTemperature);
        m_powerProperty->setMax(it->maxPower);
    }

    m_powerProperty->setUnit(KSysGuard::UnitWatt);
}

void LinuxNvidiaGpu::onDataReceived(const NvidiaSmiProcess::GpuData &data)
{
    if (data.index != m_index) {
        return;
    }

    m_usageProperty->setValue(data.usage);
    m_usedVramProperty->setValue(mbToBytes(data.memoryUsed));
    m_coreFrequencyProperty->setValue(data.coreFrequency);
    m_memoryFrequencyProperty->setValue(data.memoryFrequency);
    m_temperatureProperty->setValue(data.temperature);
    m_powerProperty->setValue(data.power);
}

#include "moc_LinuxNvidiaGpu.cpp"
