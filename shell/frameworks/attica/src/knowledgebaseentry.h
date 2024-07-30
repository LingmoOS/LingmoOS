/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ATTICA_KNOWLEDGEBASEENTRY_H
#define ATTICA_KNOWLEDGEBASEENTRY_H

#include "attica_export.h"

#include <QDateTime>
#include <QMap>
#include <QSharedDataPointer>

#include <QUrl>

namespace Attica
{

/**
 * @class KnowledgeBaseEntry knowledgebaseentry.h <Attica/KnowledgeBaseEntry>
 *
 * Represents a knowledge base entry.
 */
class ATTICA_EXPORT KnowledgeBaseEntry
{
public:
    typedef QList<KnowledgeBaseEntry> List;
    class Parser;

    KnowledgeBaseEntry();
    KnowledgeBaseEntry(const KnowledgeBaseEntry &other);
    KnowledgeBaseEntry &operator=(const KnowledgeBaseEntry &other);
    ~KnowledgeBaseEntry();

    void setId(QString id);
    QString id() const;

    void setContentId(int id);
    int contentId() const;

    void setUser(const QString &user);
    QString user() const;

    void setStatus(const QString &status);
    QString status() const;

    void setChanged(const QDateTime &changed);
    QDateTime changed() const;

    void setName(const QString &name);
    QString name() const;

    void setDescription(const QString &description);
    QString description() const;

    void setAnswer(const QString &answer);
    QString answer() const;

    void setComments(int comments);
    int comments() const;

    void setDetailPage(const QUrl &detailPage);
    QUrl detailPage() const;

    void addExtendedAttribute(const QString &key, const QString &value);
    QString extendedAttribute(const QString &key) const;

    QMap<QString, QString> extendedAttributes() const;

    bool isValid() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

#endif
