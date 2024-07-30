/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_FORUM_H
#define ATTICA_FORUM_H

#include "attica_export.h"

#include "topic.h"

#include <QDateTime>
#include <QSharedDataPointer>
#include <QUrl>

namespace Attica
{

/**
 * @class Forum forum.h <Attica/Forum>
 *
 * Represents a forum.
 */
class ATTICA_EXPORT Forum
{
public:
    typedef QList<Forum> List;
    class Parser;

    Forum();
    Forum(const Forum &other);
    Forum &operator=(const Forum &other);
    ~Forum();

    void setId(const QString &id);
    QString id() const;

    void setName(const QString &name);
    QString name() const;

    void setDescription(const QString &description);
    QString description() const;

    void setDate(const QDateTime &date);
    QDateTime date() const;

    void setIcon(const QUrl &icon);
    QUrl icon() const;

    void setChildCount(const int childCount);
    int childCount() const;

    void setTopics(const int topics);
    int topics() const;

    void setChildren(QList<Forum> comments);
    QList<Forum> children() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
