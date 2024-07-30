/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "attachmentclient.h"

#include <QVariant>

namespace Bugzilla
{
QList<int> AttachmentClient::createAttachment(KJob *kjob)
{
    auto *job = qobject_cast<APIJob *>(kjob);

    auto ary = job->object().value(QStringLiteral("ids")).toArray();
    // It's unclear if this can happen. When the ids would be empty there was
    // an error, and when there was an error the API should have sent an error.
    Q_ASSERT(ary.size() > 0);

    QList<int> list;
    for (auto ids : ary) {
        bool ok = false;
        list.append(ids.toVariant().toInt(&ok));
        Q_ASSERT(ok);
    }

    return list;
}

KJob *AttachmentClient::createAttachment(int bugId, const NewAttachment &attachment)
{
    return m_connection.post(QStringLiteral("/bug/%1/attachment").arg(bugId), attachment.toJson());
}

} // namespace Bugzilla
