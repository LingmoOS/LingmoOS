/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef NEWATTACHMENT_H
#define NEWATTACHMENT_H

#include "jsoncommand.h"

namespace Bugzilla
{
class NewAttachment : public JsonCommand
{
    Q_OBJECT
    BUGZILLA_MEMBER_PROPERTY(QList<int>, ids);
    BUGZILLA_MEMBER_PROPERTY(QString, data);
    BUGZILLA_MEMBER_PROPERTY(QString, file_name);
    BUGZILLA_MEMBER_PROPERTY(QString, summary);
    BUGZILLA_MEMBER_PROPERTY(QString, content_type);
    BUGZILLA_MEMBER_PROPERTY(QString, comment);
    BUGZILLA_MEMBER_PROPERTY(bool, is_patch) = false;
    BUGZILLA_MEMBER_PROPERTY(bool, is_private) = false;

    // flags property is not supported at this time

public:
    virtual QVariantHash toVariantHash() const override;
};

} // namespace Bugzilla

#endif // NEWATTACHMENT_H
