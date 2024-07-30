/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FREEBSDBACKEND_H
#define FREEBSDBACKEND_H

#include "backend.h"

#include <QList>

#include <kvm.h> // can't forward declare typedefed kvm_t

namespace KSysGuard
{
    class SensorProperty;
}

class FreeBsdMemoryBackend : public MemoryBackend {
public:
    FreeBsdMemoryBackend(KSysGuard::SensorContainer *container);
    void update() override;
private:
    void makeSensors() override;
    unsigned long long pagesToBytes(uint32_t pages);

    unsigned int m_pageSize;
    kvm_t *m_kd;
    QList<KSysGuard::SensorProperty *> m_sysctlSensors;
};

#endif
