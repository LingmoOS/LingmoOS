/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "commentclient.h"

#include <QVariantHash>

namespace Bugzilla
{
QList<Comment::Ptr> CommentClient::getFromBug(KJob *kjob) const
{
    auto *job = qobject_cast<APIJob *>(kjob);
    QJsonObject bugs = job->object().value(QStringLiteral("bugs")).toObject();

    // The API should never return anything other than the single bug we asked for.
    Q_ASSERT(bugs.keys().size() == 1);

    QJsonObject bug = bugs.value(bugs.keys().at(0)).toObject();
    QJsonArray comments = bug.value(QStringLiteral("comments")).toArray();

    QList<Comment::Ptr> list;
    for (auto it = comments.constBegin(); it != comments.constEnd(); ++it) {
        list.append(new Comment((*it).toObject().toVariantHash()));
    }

    return list;
}

KJob *CommentClient::getFromBug(int bugId)
{
    return m_connection.get(QStringLiteral("/bug/%1/comment").arg(QString::number(bugId)));
}

} // namespace Bugzilla
