/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KCalendarCore/CalendarPluginLoader>

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KCalendarCoreQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

void KCalendarCoreQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterSingletonType(uri, 1, 0, "CalendarPluginLoader", [](QQmlEngine *, QJSEngine *jsEngine) -> QJSValue {
        return jsEngine->toScriptValue(KCalendarCore::CalendarPluginLoader());
    });
}

#include "kcalendarcoreqmlplugin.moc"
