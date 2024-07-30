/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dummyinhibitionjob.h"
#include "dummyinhibition.h"

using namespace Solid;

DummyInhibitionJob::DummyInhibitionJob(Power::InhibitionTypes inhibitions, const QString &description, QObject *parent)
    : AbstractInhibitionJob(parent)
    , m_inhibitions(inhibitions)
    , m_description(description)
{
}

void DummyInhibitionJob::doStart()
{
    emitResult();
}

Inhibition *DummyInhibitionJob::inhibition()
{
    auto dummyInhibition = new DummyInhibition;
    dummyInhibition->inhibitions = m_inhibitions;
    dummyInhibition->description = m_description;

    return new Inhibition(dummyInhibition);
}

#include "moc_dummyinhibitionjob.cpp"
