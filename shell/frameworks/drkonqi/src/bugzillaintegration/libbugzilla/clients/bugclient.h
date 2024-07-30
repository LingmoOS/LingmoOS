/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BUGCLIENT_H
#define BUGCLIENT_H

#include "clientbase.h"
#include "commands/bugsearch.h"
#include "commands/bugupdate.h"
#include "commands/newbug.h"

#include <models/bug.h>

namespace Bugzilla
{
class BugClient : public ClientBase
{
public:
    using ClientBase::ClientBase;

    QList<Bug::Ptr> search(KJob *kjob) const;
    KJob *search(const BugSearch &search);

    qint64 create(KJob *kjob) const;
    KJob *create(const NewBug &bug);

    qint64 update(KJob *kjob) const;
    KJob *update(qint64 bugId, BugUpdate &bug);
};

} // namespace Bugzilla

#endif // BUGCLIENT_H
