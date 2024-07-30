/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KContacts/AddressFormat>

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KContactsQmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) override;
};

void KContactsQmlPlugin::registerTypes(const char *uri)
{
    qmlRegisterSingletonType(uri, 1, 0, "AddressFormatRepository", [](QQmlEngine *, QJSEngine *jsEngine) -> QJSValue {
        return jsEngine->toScriptValue(KContacts::AddressFormatRepository());
    });
}

#include "kcontactsqmlplugin.moc"
