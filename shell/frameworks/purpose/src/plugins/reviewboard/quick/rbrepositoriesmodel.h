/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef RBREPOSITORIESMODEL_H
#define RBREPOSITORIESMODEL_H

#include <KJob>
#include <QAbstractListModel>
#include <QUrl>

class RepositoriesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl server READ server WRITE setServer)
public:
    RepositoriesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void refresh();

    QUrl server() const
    {
        return m_server;
    }
    void setServer(const QUrl &server)
    {
        if (m_server != server) {
            m_server = server;
            refresh();
        }
    }

    void receivedProjects(KJob *j);
    Q_SCRIPTABLE int findRepository(const QString &name);

Q_SIGNALS:
    void repositoriesChanged();

private:
    struct Value {
        QVariant name;
        QVariant path;
        bool operator<(const Value &v1) const
        {
            return name.toString() < v1.name.toString();
        }
    };
    QList<Value> m_values;
    QUrl m_server;
};

#endif
