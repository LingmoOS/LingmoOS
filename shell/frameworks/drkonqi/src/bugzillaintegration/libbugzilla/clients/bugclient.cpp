/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "bugclient.h"

namespace Bugzilla
{
QList<Bug::Ptr> BugClient::search(KJob *kjob) const
{
    auto *job = qobject_cast<APIJob *>(kjob);

    auto ary = job->object().value(QStringLiteral("bugs")).toArray();

    QList<Bug::Ptr> list;
    for (auto bug : ary) {
        list.append(Bug::Ptr(new Bug(bug.toObject().toVariantHash())));
    }

    return list;
}

KJob *BugClient::search(const BugSearch &search)
{
    return m_connection.get(QStringLiteral("/bug"), search.toQuery());
}

qint64 BugClient::create(KJob *kjob) const
{
    auto *job = qobject_cast<APIJob *>(kjob);

    qint64 ret = job->object().value(QStringLiteral("id")).toInt(-1);
    Q_ASSERT(ret != -1);
    return ret;
}

KJob *BugClient::create(const NewBug &bug)
{
    return m_connection.post(QStringLiteral("/bug"), bug.toJson());
}

qint64 BugClient::update(KJob *kjob) const
{
    auto *job = qobject_cast<APIJob *>(kjob);

    auto ary = job->object().value(QStringLiteral("bugs")).toArray();
    // It's unclear if this can happen. When the ids would be empty there was
    // an error, and when there was an error the API should have sent an error.
    Q_ASSERT(ary.size() == 1);

    int value = ary.at(0).toObject().value(QStringLiteral("id")).toInt(-1);
    Q_ASSERT(value != -1);

    return value;
}

KJob *BugClient::update(qint64 bugId, BugUpdate &bug)
{
    return m_connection.put(QStringLiteral("/bug/%1").arg(bugId), bug.toJson());
}

} // namespace Bugzilla
