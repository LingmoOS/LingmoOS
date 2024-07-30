/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dummyinhibition.h"

using namespace Solid;

DummyInhibition::DummyInhibition(QObject *parent)
    : AbstractInhibition(parent)
    , m_state(Inhibition::Started)
{
}

DummyInhibition::~DummyInhibition()
{
    stop();
}

void DummyInhibition::start()
{
    if (m_state == Inhibition::Started) {
        return;
    }

    m_state = Inhibition::Started;
    Q_EMIT stateChanged(m_state);
}

void DummyInhibition::stop()
{
    if (m_state == Inhibition::Stopped) {
        return;
    }

    m_state = Inhibition::Stopped;
    Q_EMIT stateChanged(m_state);
}

Inhibition::State DummyInhibition::state() const
{
    return m_state;
}

#include "moc_dummyinhibition.cpp"
