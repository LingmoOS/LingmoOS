/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "inhibition.h"
#include "backends/abstractinhibition.h"

using namespace Solid;

class Solid::InhibitionPrivate
{
public:
    AbstractInhibition *backendObject;
};

Inhibition::Inhibition(AbstractInhibition *backend, QObject *parent)
    : QObject(parent)
    , d_ptr(new InhibitionPrivate)
{
    d_ptr->backendObject = backend;
    connect(d_ptr->backendObject, &AbstractInhibition::stateChanged, this, &Inhibition::stateChanged);
}

Inhibition::~Inhibition()
{
    delete d_ptr->backendObject;
    delete d_ptr;
}

void Inhibition::start()
{
    d_ptr->backendObject->start();
}

void Inhibition::stop()
{
    d_ptr->backendObject->stop();
}

Inhibition::State Inhibition::state() const
{
    return d_ptr->backendObject->state();
}

#include "moc_inhibition.cpp"
