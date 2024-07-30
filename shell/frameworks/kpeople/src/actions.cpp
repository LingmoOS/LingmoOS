/*
    SPDX-FileCopyrightText: 2015 Aleix Pol i Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "actions.h"
#include "backends/abstractpersonaction.h"
#include "kpeople_debug.h"
#include <KPeople/PersonData>

#include <KPluginFactory>
#include <KPluginMetaData>

#include <QAction>

namespace KPeople
{
static QList<AbstractPersonAction *> actionsPlugins()
{
    QList<AbstractPersonAction *> actionPlugins;
    const QList<KPluginMetaData> personPluginList = KPluginMetaData::findPlugins(QStringLiteral("kpeople/actions"));
    for (const KPluginMetaData &data : personPluginList) {
        auto pluginResult = KPluginFactory::instantiatePlugin<AbstractPersonAction>(data);
        if (pluginResult) {
            qCDebug(KPEOPLE_LOG) << "found plugin" << data.fileName();
            AbstractPersonAction *plugin = pluginResult.plugin;
            actionPlugins << plugin;
        } else {
            qCDebug(KPEOPLE_LOG) << "could not load plugin" << data.fileName() << pluginResult.errorText;
        }
    }

    return actionPlugins;
}

QList<QAction *> actionsForPerson(const QString &contactUri, QObject *parent)
{
    PersonData person(contactUri);

    QList<QAction *> actions;
    const auto lst = actionsPlugins();
    for (KPeople::AbstractPersonAction *plugin : lst) {
        actions << plugin->actionsForPerson(person, parent);
    }

    return actions;
}

}

#include "moc_actions.cpp"
