/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef REVIEWSLISTMODEL_H
#define REVIEWSLISTMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QUrl>

class KJob;

class ReviewsListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl server READ server WRITE setServer)
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(QString status READ status WRITE setStatus)
    Q_PROPERTY(QString repository READ repository WRITE setRepository)
public:
    ReviewsListModel(QObject *parent = nullptr);

    void refresh();

    QVariant data(const QModelIndex &idx, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    QUrl server() const
    {
        return m_server;
    }
    QString username() const
    {
        return m_username;
    }
    QString status() const
    {
        return m_status;
    }
    QString repository() const
    {
        return m_repository;
    }

    void setServer(const QUrl &server)
    {
        if (m_server != server) {
            m_server = server;
            refresh();
        }
    }

    void setUsername(const QString &username)
    {
        if (m_username != username) {
            m_username = username;
            refresh();
        }
    }

    void setStatus(const QString &status)
    {
        if (m_status != status) {
            m_status = status;
            refresh();
        }
    }

    void setRepository(const QString &repository)
    {
        if (m_repository != repository) {
            m_repository = repository;
            refresh();
        }
    }

    void receivedReviews(KJob *job);
    Q_SCRIPTABLE QVariant get(int row, const QByteArray &role);

private:
    struct Value {
        QVariant summary;
        QVariant id;
    };
    QList<Value> m_values;

    QUrl m_server;
    QString m_username;
    QString m_status;
    QString m_repository;
};

#endif
