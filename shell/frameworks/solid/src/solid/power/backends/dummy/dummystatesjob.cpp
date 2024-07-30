/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dummystatesjob.h"
#include <power.h>

using namespace Solid;

void DummyStatesJob::doStart()
{
    emitResult();
}

Power::InhibitionTypes DummyStatesJob::states() const
{
    return Power::Shutdown | Power::Sleep;
}

#include "moc_dummystatesjob.cpp"
