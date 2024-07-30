/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LINUXBACKEND_H
#define LINUXBACKEND_H

#include "backend.h"

class LinuxMemoryBackend : public MemoryBackend {
public:
    LinuxMemoryBackend(KSysGuard::SensorContainer *container);
    void update() override;
};

#endif
