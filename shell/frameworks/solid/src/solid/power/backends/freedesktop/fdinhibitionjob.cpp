/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fdinhibitionjob.h"
#include "fdinhibition.h"

#include <unistd.h>

using namespace Solid;

FdInhibitionJob::FdInhibitionJob(Power::InhibitionTypes inhibitions, const QString &description, QObject *parent)
    : AbstractInhibitionJob(parent)
    , m_inhibition(new FdInhibition(inhibitions, description))
{
}

FdInhibitionJob::~FdInhibitionJob()
{
}

void FdInhibitionJob::doStart()
{
    connect(m_inhibition, &FdInhibition::stateChanged, this, &FdInhibitionJob::stateChanged);
    m_inhibition->start();
}

void FdInhibitionJob::stateChanged(Inhibition::State state)
{
    Q_UNUSED(state);

    disconnect(m_inhibition, nullptr, this, nullptr);
    emitResult();
}

Inhibition *FdInhibitionJob::inhibition()
{
    return new Inhibition(m_inhibition);
}

#include "moc_fdinhibitionjob.cpp"
