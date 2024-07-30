/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BUGFIELDCLIENT_H
#define BUGFIELDCLIENT_H

#include "clientbase.h"
#include "models/bugfield.h"

namespace Bugzilla
{
class BugFieldClient : public ClientBase
{
public:
    using ClientBase::ClientBase;

    KJob *getFields(const QString &idOrName = QString());
    QList<BugField::Ptr> getFields(KJob *kjob) const;

    KJob *getField(const QString &idOrName);
    /// Ptr may be null if the idOrName matched nothing!
    BugField::Ptr getField(KJob *kjob) const;
};

} // namespace Bugzilla

#endif // BUGFIELDCLIENT_H
