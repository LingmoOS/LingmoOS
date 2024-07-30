/*
    SPDX-FileCopyrightText: 2010 Rafael Fernández López <ereslibre@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevprocessor.h"

#include "../shared/cpufeatures.h"
#include "cpuinfo.h"
#include "udevdevice.h"

#include <QFile>

using namespace Solid::Backends::UDev;

Processor::Processor(UDevDevice *device)
    : DeviceInterface(device)
    , m_canChangeFrequency(NotChecked)
    , m_maxSpeed(-1)
{
}

Processor::~Processor()
{
}

int Processor::number() const
{
    // There's a subtle assumption here: suppose the system's ACPI
    // supports more processors/cores than are installed, and so udev reports
    // 4 cores when there are 2, say.  Will the processor numbers (in
    // /proc/cpuinfo, in particular) always match the sysfs device numbers?
    return m_device->deviceNumber();
}

int Processor::maxSpeed() const
{
    if (m_maxSpeed == -1) {
        QFile cpuMaxFreqFile(m_device->deviceName() + prefix() + "/cpufreq/cpuinfo_max_freq");
        if (cpuMaxFreqFile.open(QIODevice::ReadOnly)) {
            QString value = cpuMaxFreqFile.readAll().trimmed();
            // cpuinfo_max_freq is in kHz
            m_maxSpeed = static_cast<int>(value.toLongLong() / 1000);
        }
        if (m_maxSpeed <= 0) {
            // couldn't get the info from /sys, try /proc instead
            m_maxSpeed = extractCurrentCpuSpeed(number());
        }
    }
    return m_maxSpeed;
}

bool Processor::canChangeFrequency() const
{
    if (m_canChangeFrequency == NotChecked) {
        /* Note that cpufreq is the right information source here, rather than
         * anything to do with throttling (ACPI T-states).  */

        m_canChangeFrequency = CannotChangeFreq;

        QFile cpuMinFreqFile(m_device->deviceName() + prefix() + "/cpufreq/cpuinfo_min_freq");
        QFile cpuMaxFreqFile(m_device->deviceName() + prefix() + "/cpufreq/cpuinfo_max_freq");
        if (cpuMinFreqFile.open(QIODevice::ReadOnly) && cpuMaxFreqFile.open(QIODevice::ReadOnly)) {
            qlonglong minFreq = cpuMinFreqFile.readAll().trimmed().toLongLong();
            qlonglong maxFreq = cpuMaxFreqFile.readAll().trimmed().toLongLong();
            if (minFreq > 0 && maxFreq > minFreq) {
                m_canChangeFrequency = CanChangeFreq;
            }
        }
    }

    return m_canChangeFrequency == CanChangeFreq;
}

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    static Solid::Processor::InstructionSets cpuextensions = Solid::Backends::Shared::cpuFeatures();

    return cpuextensions;
}

QString Processor::prefix() const
{
    QLatin1String sysPrefix("/sysdev");
    if (QFile::exists(m_device->deviceName() + sysPrefix)) {
        return sysPrefix;
    }

    return QString();
}

#include "moc_udevprocessor.cpp"
