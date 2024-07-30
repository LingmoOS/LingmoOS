/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "power.h"
#include "acpluggedjob.h"
#include "inhibition.h"
#include "inhibitionjob.h"
#include "powerbackendloader.h"
#include "requeststatejob.h"
#include "statesjob.h"

#include "backends/powernotifier.h"

Q_GLOBAL_STATIC(Solid::Power, globalPowerObject)

using namespace Solid;

class Power::Private
{
public:
    PowerNotifier *notifier;
};

Power *Power::self()
{
    return globalPowerObject;
}

Power::Power(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    qRegisterMetaType<Solid::Inhibition::State>("Inhibition::State");
    d->notifier = PowerBackendLoader::notifier();
    connect(d->notifier, &PowerNotifier::acPluggedChanged, this, &Power::acPluggedChanged);
    connect(d->notifier, &PowerNotifier::aboutToSuspend, this, &Power::aboutToSuspend);
    connect(d->notifier, &PowerNotifier::resumeFromSuspend, this, &Power::resumeFromSuspend);
}

AcPluggedJob *Power::isAcPlugged(QObject *parent)
{
    return new AcPluggedJob(parent);
}

InhibitionJob *Power::inhibit(Power::InhibitionTypes states, const QString &description, QObject *parent)
{
    InhibitionJob *job = new InhibitionJob(parent);
    job->setInhibitions(states);
    job->setDescription(description);

    return job;
}

StatesJob *Power::supportedStates(QObject *parent)
{
    return new StatesJob(parent);
}

RequestStateJob *Power::requestState(Power::InhibitionType state, QObject *parent)
{
    auto job = new RequestStateJob(parent);
    job->setState(state);

    return job;
}

#include "moc_power.cpp"
