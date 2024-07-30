// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "kpluginfactory.h"

class StaticSimplePluginClass : public QObject
{
    Q_OBJECT

public:
    // Next to the assertion below, ensure that we have no ambiguity!
    explicit StaticSimplePluginClass(QObject *parent, const KPluginMetaData &data = {})
        : QObject(parent)
    {
        // We have added a default arg, but KPluginFactory should still provide the valid metadata instead of the default one
        Q_ASSERT(data.isValid());
    }
};

K_PLUGIN_CLASS_WITH_JSON(StaticSimplePluginClass, "data/jsonplugin.json")

#include "statickpluginclass.moc"
