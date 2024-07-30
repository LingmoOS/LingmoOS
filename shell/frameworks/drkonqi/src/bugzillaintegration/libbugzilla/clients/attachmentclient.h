/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTACHMENTCLIENT_H
#define ATTACHMENTCLIENT_H

#include "clientbase.h"
#include "commands/newattachment.h"

namespace Bugzilla
{
class AttachmentClient : public ClientBase
{
public:
    using ClientBase::ClientBase;

    /// Attach to a bug. @returns list of bugs that were attached to.
    QList<int> createAttachment(KJob *kjob);
    KJob *createAttachment(int bugId, const NewAttachment &attachment);
};

} // namespace Bugzilla

#endif // ATTACHMENTCLIENT_H
