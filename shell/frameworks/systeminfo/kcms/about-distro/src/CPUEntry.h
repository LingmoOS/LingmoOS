/*
    SPDX-FileCopyrightText: 2012-2021 Harald Sitter <sitter@kde.org>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CPUENTRY_H
#define CPUENTRY_H

#include <solid/device.h>

#include "Entry.h"

class CPUEntry : public Entry
{
public:
    CPUEntry();
    QString localizedValue(Language language = Language::System) const override;

private:
    const QList<Solid::Device> m_cpus = Solid::Device::listFromType(Solid::DeviceInterface::Processor);
};

#endif // CPUENTRY_H
