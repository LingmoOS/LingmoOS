/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "powerbackendloader.h"
#include "backends/abstractacpluggedjob.h"
#include "backends/dummy/dummyacpluggedjob.h"
#include "backends/dummy/dummyinhibitionjob.h"
#include "backends/dummy/dummypowernotifier.h"
#include "backends/dummy/dummyrequeststatejob.h"
#include "backends/dummy/dummystatesjob.h"
#include "backends/freedesktop/fdacpluggedjob.h"
#include "backends/freedesktop/fdinhibitionjob.h"
#include "backends/freedesktop/fdpowernotifier.h"

using namespace Solid;

AbstractAcPluggedJob *PowerBackendLoader::AcPluggedJob()
{
    if (qgetenv("SOLID_POWER_BACKEND") == "DUMMY") {
        return new DummyAcPluggedJob();
    }
    return new FDAcPluggedJob();
}

AbstractInhibitionJob *PowerBackendLoader::addInhibitionJob(Power::InhibitionTypes inhibitions, const QString &description)
{
    if (qgetenv("SOLID_POWER_BACKEND") == "DUMMY") {
        return new DummyInhibitionJob(inhibitions, description);
    }
    return new FdInhibitionJob(inhibitions, description);
}

AbstractStatesJob *PowerBackendLoader::statesJob()
{
    return new DummyStatesJob();
}

AbstractRequestStateJob *PowerBackendLoader::requestState()
{
    return new DummyRequestStateJob();
}

PowerNotifier *PowerBackendLoader::notifier()
{
    if (qgetenv("SOLID_POWER_BACKEND") == "DUMMY") {
        return new DummyPowerNotifier();
    }
    return new FDPowerNotifier();
}
