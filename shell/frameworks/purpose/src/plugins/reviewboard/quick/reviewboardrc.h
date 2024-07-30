/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REVIEWBOARDRC_H
#define REVIEWBOARDRC_H

#include <QJsonObject>
#include <QObject>
#include <QUrl>

class ReviewboardRC : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl path READ path WRITE setPath)
    Q_PROPERTY(QUrl server READ server NOTIFY dataChanged)
    Q_PROPERTY(QString repository READ repository NOTIFY dataChanged)
    Q_PROPERTY(QJsonObject extraData READ extraData NOTIFY dataChanged)
public:
    ReviewboardRC(QObject *parent = nullptr);

    void setPath(const QUrl &path);

    QUrl path() const
    {
        return m_path;
    }
    QUrl server() const
    {
        return m_server;
    }
    QString repository() const
    {
        return m_repository;
    }
    QJsonObject extraData() const
    {
        return m_extraData;
    }

Q_SIGNALS:
    void dataChanged();

private:
    void addExtraData(const QString &key, const QString &value);

    QUrl m_path;
    QUrl m_server;
    QString m_repository;
    QJsonObject m_extraData;
};

#endif
