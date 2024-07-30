/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef ABSTRACT_PERSON_ACTION_H
#define ABSTRACT_PERSON_ACTION_H

#include <QAction>
#include <QObject>

#include <KPeople/PersonData>
#include <kpeoplebackend/kpeoplebackend_export.h>

namespace KPeople
{
class PersonData;

class KPEOPLEBACKEND_EXPORT AbstractPersonAction : public QObject
{
    Q_OBJECT
public:
    explicit AbstractPersonAction(QObject *parent);
    /** A list of actions that can be executed for a contact
     * Disabled actions should not be added
     *
     * @param data passes the person we're creating the actions for
     * @param parent parent object for newly created actions
     */
    virtual QList<QAction *> actionsForPerson(const PersonData &data, QObject *parent) const = 0;
};

}

#endif
