/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BUGUPDATE_H
#define BUGUPDATE_H

#include "jsoncommand.h"

namespace Bugzilla
{
class BugUpdateCC : public JsonCommand
{
    Q_OBJECT
    BUGZILLA_MEMBER_PROPERTY(QStringList, add);
    BUGZILLA_MEMBER_PROPERTY(QStringList, remove);

public:
    using JsonCommand::JsonCommand;
};

class BugUpdate : public JsonCommand
{
    Q_OBJECT
    BUGZILLA_MEMBER_PROPERTY(BugUpdateCC *, cc) = new BugUpdateCC(this);
};

} // namespace Bugzilla

#endif // BUGUPDATE_H
