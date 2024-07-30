/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BUGSEARCH_H
#define BUGSEARCH_H

#include "querycommand.h"

namespace Bugzilla
{
class BugSearch : public QueryCommand
{
    Q_OBJECT
    BUGZILLA_MEMBER_PROPERTY(QStringList, products);
    BUGZILLA_MEMBER_PROPERTY(QString, severity);
    BUGZILLA_MEMBER_PROPERTY(QString, creationTime);
    BUGZILLA_MEMBER_PROPERTY(qint64, id) = -1;
    BUGZILLA_MEMBER_PROPERTY(qint64, limit) = -1;
    BUGZILLA_MEMBER_PROPERTY(qint64, offset) = -1;
    BUGZILLA_MEMBER_PROPERTY(QString, longdesc);
    BUGZILLA_MEMBER_PROPERTY(QStringList, order);

public:
    virtual Query toQuery() const override;
};

} // namespace Bugzilla

#endif // BUGSEARCH_H
