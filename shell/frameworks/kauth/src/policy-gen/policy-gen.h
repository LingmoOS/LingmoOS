/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _POLICY_GEN_H_
#define _POLICY_GEN_H_

#include <QHash>
#include <QMap>
#include <QString>

struct Action {
    QString name;

    QMap<QString, QString> descriptions;
    QMap<QString, QString> messages;

    QString policy;
    QString policyInactive;
    QString persistence;
};

extern void output(const QList<Action> &actions, const QMap<QString, QString> &domain);

#endif
