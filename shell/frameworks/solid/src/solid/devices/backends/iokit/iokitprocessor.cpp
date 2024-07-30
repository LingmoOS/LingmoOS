/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "iokitprocessor.h"
#include "iokitdevice.h"

#include <qdebug.h>

#include <errno.h>
#include <sys/sysctl.h>
#include <sys/types.h>

#include "../shared/cpufeatures.h"

// from cfhelper.cpp
extern bool q_sysctlbyname(const char *name, QString &result);

using namespace Solid::Backends::IOKit;

Processor::Processor(IOKitDevice *device)
    : DeviceInterface(device)
{
    // IOKitDevice parent(device->parentUdi());
}

Processor::~Processor()
{
}

int Processor::number() const
{
    return m_device->property(QLatin1String("IOCPUNumber")).toInt();
}

int Processor::maxSpeed() const
{
    uint64_t freq = 0;
    size_t size = sizeof(freq);

    if (sysctlbyname("hw.cpufrequency", &freq, &size, nullptr, 0) < 0) {
        qWarning() << "sysctl error reading hw.cpufrequency:" << strerror(errno);
        return 0;
    } else {
        return int(freq / 1000000);
    }
}

bool Processor::canChangeFrequency() const
{
    uint64_t minFreq = 0, maxFreq = 0;
    size_t size = sizeof(uint64_t);

    if (sysctlbyname("hw.cpufrequency_min", &minFreq, &size, nullptr, 0) == 0 //
        && sysctlbyname("hw.cpufrequency_max", &maxFreq, &size, nullptr, 0) == 0) {
        return maxFreq > minFreq;
    }
    return false;
}

Solid::Processor::InstructionSets Processor::instructionSets() const
{
    // use sysctlbyname() and "machdep.cpu.features" + "machdep.cpu.extfeatures"
    static Solid::Processor::InstructionSets cpuextensions = Solid::Backends::Shared::cpuFeatures();

    return cpuextensions;
}

QString Processor::vendor()
{
    QString qVendor;
    q_sysctlbyname("machdep.cpu.vendor", qVendor);
    return qVendor;
}

QString Processor::product()
{
    QString product;
    q_sysctlbyname("machdep.cpu.brand_string", product);
    return product;
}

#include "moc_iokitprocessor.cpp"
