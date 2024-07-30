/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NEWBUG_H
#define NEWBUG_H

#include "jsoncommand.h"

namespace Bugzilla
{
class NewBug : public JsonCommand
{
    Q_OBJECT
    BUGZILLA_MEMBER_PROPERTY(QString, product);
    BUGZILLA_MEMBER_PROPERTY(QString, component);
    BUGZILLA_MEMBER_PROPERTY(QString, summary); // aka shortdesc
    BUGZILLA_MEMBER_PROPERTY(QString, version);
    BUGZILLA_MEMBER_PROPERTY(QString, description);
    BUGZILLA_MEMBER_PROPERTY(QString, op_sys);
    BUGZILLA_MEMBER_PROPERTY(QString, platform);
    BUGZILLA_MEMBER_PROPERTY(QString, priority);
    BUGZILLA_MEMBER_PROPERTY(QString, severity);
    BUGZILLA_MEMBER_PROPERTY(QStringList, keywords); // not documented but also supported
public:
    // only needed because we impl the copy ctor, otherwise this could be `using`
    explicit NewBug(QObject *parent = nullptr);
    NewBug(const NewBug &other);
};

} // namespace Bugzilla

#endif // NEWBUG_H
