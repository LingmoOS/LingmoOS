/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dummyacpluggedjob.h"

using namespace Solid;

void DummyAcPluggedJob::doStart()
{
    emitResult();
}

bool DummyAcPluggedJob::isPlugged() const
{
    return true;
}

#include "moc_dummyacpluggedjob.cpp"
