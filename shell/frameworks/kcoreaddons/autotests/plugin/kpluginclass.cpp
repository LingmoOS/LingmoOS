// SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

#include "kpluginfactory.h"
#include "plugins.h"

class SimplePluginClass : public MyPlugin
{
    Q_OBJECT
public:
    explicit SimplePluginClass(QObject *parent, const QVariantList &args)
        : MyPlugin(parent)
    {
        setProperty("arg", args.isEmpty() ? QVariant() : args.first());
    }
};
class SimplePluginClass2 : public MyPlugin2
{
    Q_OBJECT
public:
    explicit SimplePluginClass2(QObject *parent)
        : MyPlugin2(parent)
    {
    }
};

K_PLUGIN_FACTORY_WITH_JSON(MyFactory, "data/jsonplugin.json", registerPlugin<SimplePluginClass>(); registerPlugin<SimplePluginClass2>();)

#include "kpluginclass.moc"
