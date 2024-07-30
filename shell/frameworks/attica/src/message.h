/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_MESSAGE_H
#define ATTICA_MESSAGE_H

#include <QDateTime>
#include <QList>
#include <QSharedDataPointer>

#include "attica_export.h"

namespace Attica
{

/**
 * @class Message message.h <Attica/Message>
 *
 * Represents a message.
 */
class ATTICA_EXPORT Message
{
public:
    typedef QList<Message> List;
    class Parser;

    enum Status {
        Unread = 0,
        Read = 1,
        Answered = 2,
    };

    Message();
    Message(const Message &other);
    Message &operator=(const Message &other);
    ~Message();

    void setId(const QString &);
    QString id() const;

    void setFrom(const QString &);
    QString from() const;

    void setTo(const QString &);
    QString to() const;

    void setSent(const QDateTime &);
    QDateTime sent() const;

    void setStatus(Status);
    Status status() const;

    void setSubject(const QString &);
    QString subject() const;

    void setBody(const QString &);
    QString body() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
