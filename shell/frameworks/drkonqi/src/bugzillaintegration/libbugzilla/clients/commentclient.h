/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef COMMENTCLIENT_H
#define COMMENTCLIENT_H

#include "clientbase.h"
#include "models/comment.h"

namespace Bugzilla
{
class CommentClient : public ClientBase
{
public:
    using ClientBase::ClientBase;

    QList<Comment::Ptr> getFromBug(KJob *kjob) const;
    KJob *getFromBug(int bugId);
};

} // namespace Bugzilla

#endif // COMMENTCLIENT_H
