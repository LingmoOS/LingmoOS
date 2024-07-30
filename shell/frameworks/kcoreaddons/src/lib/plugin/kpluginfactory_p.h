/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINFACTORY_P_H
#define KPLUGINFACTORY_P_H

#include "kpluginfactory.h"
#include <KPluginMetaData>

class KPluginFactoryPrivate
{
public:
    using PluginWithMetadata = QPair<const QMetaObject *, KPluginFactory::CreateInstanceWithMetaDataFunction>;
    KPluginMetaData metaData;
    std::vector<PluginWithMetadata> createInstanceWithMetaDataHash;
};

#endif // KPLUGINFACTORY_P_H
