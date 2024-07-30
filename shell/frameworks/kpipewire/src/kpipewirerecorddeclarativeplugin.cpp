/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kpipewirerecorddeclarativeplugin.h"

#include "pipewirerecord.h"
#include <qqml.h>

void KPipewireRecordDeclarativePlugin::registerTypes(const char *uri)
{
    qmlRegisterType<PipeWireRecord>(uri, 0, 1, "PipeWireRecord");
}

#include "moc_kpipewirerecorddeclarativeplugin.cpp"
