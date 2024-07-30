/*
    Persons Model
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef PEOPLEQMLPLUGIN_H
#define PEOPLEQMLPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include "personsmodel.h"
#include <KPeople/PersonPluginManager>
#include <actions.h>
#include <personactionsmodel_p.h>
#include <personssortfilterproxymodel.h>

class DeclarativePersonPluginManager : public QObject
{
    Q_OBJECT
    QML_SINGLETON
    QML_NAMED_ELEMENT(PersonPluginManager)

public:
    Q_SCRIPTABLE bool addContact(const QVariantMap &properties)
    {
        return KPeople::PersonPluginManager::addContact(properties);
    }
    Q_SCRIPTABLE bool deleteContact(const QString &uri)
    {
        return KPeople::PersonPluginManager::deleteContact(uri);
    }
};

namespace ActionType
{
Q_NAMESPACE
QML_ELEMENT

enum ActionType {
    TextChatAction = KPeople::TextChatAction,
    AudioCallAction = KPeople::AudioCallAction,
    VideoCallAction = KPeople::VideoCallAction,
    SendEmailAction = KPeople::SendEmailAction,
    SendFileAction = KPeople::SendFileAction,
    OtherAction = KPeople::OtherAction,
};
Q_ENUM_NS(ActionType)
};

class PeopleQMLPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")
public:
    void initializeEngine(QQmlEngine *engine, const char *uri) override;
    void registerTypes(const char *uri) override;
};

struct PersonDataForeign {
    Q_GADGET
    QML_FOREIGN(KPeople::PersonData)
    QML_ANONYMOUS
};

struct PersonsModelForeign {
    Q_GADGET
    QML_FOREIGN(KPeople::PersonsModel)
    QML_NAMED_ELEMENT(PersonsModel)
};

class PersonsSortFilterProxyModelForeign
{
    Q_GADGET
    QML_FOREIGN(KPeople::PersonsSortFilterProxyModel)
    QML_NAMED_ELEMENT(PersonsSortFilterProxyModel)
};

#endif // PEOPLEQMLPLUGIN_H
