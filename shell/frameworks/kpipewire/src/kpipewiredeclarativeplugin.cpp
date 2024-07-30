/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kpipewiredeclarativeplugin.h"

#include "pipewiresourceitem.h"
#include "pipewirerecord.h"

void KPipewireDeclarativePlugin::registerTypes(const char *uri)
{
    qmlRegisterType<PipeWireSourceItem>(uri, 0, 1, "PipeWireSourceItem");
}

#include "moc_kpipewiredeclarativeplugin.cpp"
