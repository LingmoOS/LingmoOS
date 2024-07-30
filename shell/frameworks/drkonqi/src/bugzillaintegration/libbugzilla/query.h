/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <QMultiMap>
#include <QString>

namespace Bugzilla
{
// Query container. Do not use QUrlQuery. Since bugzilla wants more encoding
// than QUrlQuery would provide by default we always store the input values
// in this Query container. Only once we are actually ready to construct
// the final request is this container converted to a QUrlQuery.
//
// NB: This class intentionally has no QUrlQuery converter function because
//   any behavior we need must be implemented in this container, not run
//   through QUrlQuery to prevent encoding confusion.
//
// QMap is used as base because order makes test assertions easier to check.
class Query : public QMultiMap<QString, QString>
{
public:
    using QMultiMap<QString, QString>::QMultiMap;

    // Compat rigging so this feels like QUrlQuery and reduces porting
    // noise for bugfix backport.
    bool hasQueryItem(const QString &key);
    void addQueryItem(const QString &key, const QString &value);

    // Don't use this to do anything other than streaming to qDebug.
    // The output is not encoded and thus not necessarily a valid URL query.
    QString toString() const;
};

} // namespace Bugzilla
