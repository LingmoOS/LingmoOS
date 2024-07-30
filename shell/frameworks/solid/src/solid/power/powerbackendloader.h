/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef POWER_BACKEND_LOADER_H
#define POWER_BACKEND_LOADER_H

#include <solid/power.h>

namespace Solid
{
class AbstractAcPluggedJob;
class AbstractInhibitionJob;
class AbstractStatesJob;
class AbstractRequestStateJob;
class PowerNotifier;
class PowerBackendLoader
{
public:
    static AbstractAcPluggedJob *AcPluggedJob();
    static AbstractInhibitionJob *addInhibitionJob(Power::InhibitionTypes inhibitions, const QString &description);
    static AbstractStatesJob *statesJob();
    static AbstractRequestStateJob *requestState();
    static PowerNotifier *notifier();
};
}

#endif // POWER_BACKEND_LOADER_H
