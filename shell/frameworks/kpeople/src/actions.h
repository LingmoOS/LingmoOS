/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KPEOPLE_ACTIONS_H
#define KPEOPLE_ACTIONS_H

#include <kpeople/kpeople_export.h>
#include <qobjectdefs.h>

#include <QList>

class QString;
class QObject;
class QAction;
namespace KPeople
{
Q_NAMESPACE_EXPORT(KPEOPLE_EXPORT)
class PersonData;

/**
 * Each action returned in the list can be one of these
 * types, however the Type is not mandatory with the action
 *
 * The type should be set as QObject property "actionType"
 */
enum ActionType {
    TextChatAction,
    AudioCallAction,
    VideoCallAction,
    SendEmailAction,
    SendFileAction,
    OtherAction = 100,
};
Q_ENUM_NS(ActionType)

/**
 * Returns a list of actions relevant to the specified @p contactUri where
 * each QAction will have @p parent passed as its parent QObject
 */
KPEOPLE_EXPORT QList<QAction *> actionsForPerson(const QString &contactUri, QObject *parent);
}

#endif // KPEOPLE_ACTIONS_H
