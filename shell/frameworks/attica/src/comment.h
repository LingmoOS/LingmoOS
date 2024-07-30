/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2010 Intel Corporation
    SPDX-FileContributor: Mateu Batle Sastre <mbatle@collabora.co.uk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_COMMENT_H
#define ATTICA_COMMENT_H

#include "attica_export.h"

#include <QDateTime>
#include <QSharedDataPointer>

#include <QUrl>

namespace Attica
{

/**
 * @class Comment comment.h <Attica/Comment>
 *
 * Represents a comment.
 */
class ATTICA_EXPORT Comment
{
public:
    typedef QList<Comment> List;
    class Parser;

    enum Type {
        ContentComment,
        ForumComment,
        KnowledgeBaseComment,
        EventComment,
    };
    static QString commentTypeToString(const Comment::Type type);

    Comment();
    Comment(const Comment &other);
    Comment &operator=(const Comment &other);
    ~Comment();

    void setId(const QString &id);
    QString id() const;

    void setSubject(const QString &subject);
    QString subject() const;

    void setText(const QString &text);
    QString text() const;

    void setChildCount(const int childCount);
    int childCount() const;

    void setUser(const QString &user);
    QString user() const;

    void setDate(const QDateTime &date);
    QDateTime date() const;

    /**
      This is for internal usage, @see Provider::setCommentScore to set scores in comments.
      @param score average comment score in scale from 0 to 100
     */
    void setScore(const int score);
    /**
      Returns score of this comment.
      @param score average comment score in scale from 0 to 100
     */
    int score() const;

    void setChildren(QList<Comment> comments);
    QList<Comment> children() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
