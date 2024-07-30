/*
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LOADAVERAGES_H
#define LOADAVERAGES_H

#include <systemstats/SensorObject.h>

class LoadAverages : public KSysGuard::SensorObject {
public:
    LoadAverages(KSysGuard::SensorContainer *parent);
    void update();
private:
    KSysGuard::SensorProperty *const average1Minute;
    KSysGuard::SensorProperty *const average5Minutes;
    KSysGuard::SensorProperty *const average15Minutes;
};

#endif
