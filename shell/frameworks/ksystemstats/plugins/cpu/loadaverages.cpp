/*
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "loadaverages.h"

#include <KLocalizedString>
#include <array>

LoadAverages::LoadAverages(KSysGuard::SensorContainer *parent)
    : KSysGuard::SensorObject(QStringLiteral("loadaverages"), i18nc("@title", "Load Averages"), parent)
    , average1Minute{new KSysGuard::SensorProperty(QStringLiteral("loadaverage1"), i18nc("@title", "Load average (1 minute)"), 0, this)}
    , average5Minutes{new KSysGuard::SensorProperty(QStringLiteral("loadaverage5"), i18nc("@title", "Load average (5 minutes)"), 0, this)}
    , average15Minutes{new KSysGuard::SensorProperty(QStringLiteral("loadaverage15"), i18nc("@title", "Load average (15 minute)"), 0, this)}
{
    average1Minute->setShortName(i18nc("@title,  Short for 'Load average (1 minute)", "Load average (1m)"));
    average5Minutes->setShortName(i18nc("@title,  Short for 'Load average (5 minutes)", "Load average (5m)"));
    average15Minutes->setShortName(i18nc("@title,  Short for 'Load average (15 minutes)", "Load average (15m)"));

    average1Minute->setDescription(i18nc("@info", "Number of jobs in the run queue averaged over 1 minute"));
    average5Minutes->setDescription(i18nc("@info", "Number of jobs in the run queue averaged over 5 minutes"));
    average15Minutes->setDescription(i18nc("@info", "Number of jobs in the run queue averaged over 15 minutes"));

    average1Minute->setUnit(KSysGuard::UnitNone);
    average5Minutes->setUnit(KSysGuard::UnitNone);
    average15Minutes->setUnit(KSysGuard::UnitNone);
}

void LoadAverages::update()
{
    if (isSubscribed()) {
        std::array<double, 3> averages;
        const int numRetrieved = getloadavg(averages.data(), 3);
        if (numRetrieved >= 1) {
            average1Minute->setValue(averages[0]);
        }
        if (numRetrieved >= 2) {
            average5Minutes->setValue(averages[1]);
        }
        if (numRetrieved == 3) {
            average15Minutes->setValue(averages[2]);
        }
    }
}
