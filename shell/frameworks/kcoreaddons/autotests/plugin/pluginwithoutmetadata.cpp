/*
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <kpluginfactory.h>

class PluginWithoutMetaData : public QObject
{
    Q_OBJECT
public:
    // Add a default arg to make sure we do not get an ambiguity compiler error
    PluginWithoutMetaData(const QObject *, const QVariantList &args = {})
        : QObject()
    {
        Q_UNUSED(args)
    };
};

K_PLUGIN_CLASS(PluginWithoutMetaData)

#include "pluginwithoutmetadata.moc"
