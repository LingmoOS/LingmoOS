/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2011 Laszlo Papp <djszapi@archlinux.us>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_TOPIC_H
#define ATTICA_TOPIC_H

#include "attica_export.h"

#include <QDateTime>
#include <QSharedDataPointer>

namespace Attica
{

/**
 * @class Topic topic.h <Attica/Topic>
 *
 * Represents a topic.
 */
class ATTICA_EXPORT Topic
{
public:
    typedef QList<Topic> List;
    class Parser;

    Topic();
    Topic(const Topic &other);
    Topic &operator=(const Topic &other);
    ~Topic();

    void setId(const QString &id);
    QString id() const;

    void setForumId(const QString &forumId);
    QString forumId() const;

    void setUser(const QString &user);
    QString user() const;

    void setDate(const QDateTime &date);
    QDateTime date() const;

    void setSubject(const QString &subject);
    QString subject() const;

    void setContent(const QString &content);
    QString content() const;

    void setComments(const int comments);
    int comments() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
