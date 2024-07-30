/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BACKEND_H
#define BACKEND_H

namespace KSysGuard {
    class SensorContainer;
    class SensorObject;
    class SensorProperty;
}

class MemoryBackend {
public:
    MemoryBackend(KSysGuard::SensorContainer *container);
    virtual ~MemoryBackend() = default;

    void initSensors();
    virtual void update() = 0;
protected:
    virtual void makeSensors();

    KSysGuard::SensorProperty *m_total = nullptr;
    KSysGuard::SensorProperty *m_used = nullptr;
    KSysGuard::SensorProperty *m_free = nullptr;
    KSysGuard::SensorProperty *m_application = nullptr;
    KSysGuard::SensorProperty *m_cache = nullptr;
    KSysGuard::SensorProperty *m_buffer = nullptr;
    KSysGuard::SensorProperty *m_swapTotal = nullptr;
    KSysGuard::SensorProperty *m_swapUsed = nullptr;
    KSysGuard::SensorProperty *m_swapFree = nullptr;
    KSysGuard::SensorObject *m_physicalObject = nullptr;
    KSysGuard::SensorObject *m_swapObject = nullptr;
};

#endif
