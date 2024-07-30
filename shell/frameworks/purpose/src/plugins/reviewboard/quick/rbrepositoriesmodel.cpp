/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rbrepositoriesmodel.h"
#include "reviewboardjobs.h"

RepositoriesModel::RepositoriesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    refresh();
}

void RepositoriesModel::refresh()
{
    if (m_server.isEmpty()) {
        beginResetModel();
        m_values.clear();
        endResetModel();
        Q_EMIT repositoriesChanged();
        return;
    }
    ReviewBoard::ProjectsListRequest *repo = new ReviewBoard::ProjectsListRequest(m_server, this);
    connect(repo, &ReviewBoard::ProjectsListRequest::finished, this, &RepositoriesModel::receivedProjects);
    repo->start();
}

QVariant RepositoriesModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.column() != 0 || idx.row() >= m_values.count()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_values[idx.row()].name;
    case Qt::ToolTipRole:
        return m_values[idx.row()].path;
    default:
        return QVariant();
    }
}

int RepositoriesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_values.count();
}

void RepositoriesModel::receivedProjects(KJob *job)
{
    if (job->error()) {
        qWarning() << "received error when fetching repositories:" << job->error() << job->errorString();

        beginResetModel();
        m_values.clear();
        endResetModel();
        Q_EMIT repositoriesChanged();
        return;
    }

    ReviewBoard::ProjectsListRequest *pl = dynamic_cast<ReviewBoard::ProjectsListRequest *>(job);

    beginResetModel();
    m_values.clear();
    const auto repositories = pl->repositories();
    for (const QVariant &repo : repositories) {
        const QVariantMap repoMap = repo.toMap();
        m_values += Value{repoMap[QStringLiteral("name")], repoMap[QStringLiteral("path")]};
    }
    std::sort(m_values.begin(), m_values.end());
    endResetModel();
    Q_EMIT repositoriesChanged();
}

int RepositoriesModel::findRepository(const QString &name)
{
    QModelIndexList idxs = match(index(0, 0), Qt::ToolTipRole, name, 1, Qt::MatchExactly);
    if (idxs.isEmpty()) {
        idxs = match(index(0, 0), Qt::DisplayRole, QUrl(name).fileName(), 1, Qt::MatchExactly);
    }
    if (!idxs.isEmpty()) {
        return idxs.first().row();
    } else {
        qWarning() << "couldn't find the repository" << name;
    }

    return -1;
}

#include "moc_rbrepositoriesmodel.cpp"
