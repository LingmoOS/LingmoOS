// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "kpluginfactory.h"

class AnotherStaticSimplePluginClass : public QObject
{
    Q_OBJECT

public:
    explicit AnotherStaticSimplePluginClass(QObject *)
    {
    }
};

K_PLUGIN_CLASS_WITH_JSON(AnotherStaticSimplePluginClass, "data/jsonplugin.json")

#include "statickpluginclass_3.moc"
