/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "rbreviewslistmodel.h"
#include "reviewboardjobs.h"

ReviewsListModel::ReviewsListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    refresh();
}

void ReviewsListModel::refresh()
{
    if (m_server.isEmpty() || m_repository.isEmpty()) {
        beginResetModel();
        m_values.clear();
        endResetModel();
        return;
    }

    ReviewBoard::ReviewListRequest *repo = new ReviewBoard::ReviewListRequest(m_server, m_username, m_status, this);
    connect(repo, &ReviewBoard::ReviewListRequest::finished, this, &ReviewsListModel::receivedReviews);
    repo->start();
}

void ReviewsListModel::receivedReviews(KJob *job)
{
    if (job->error() != 0) {
        qWarning() << "error review list" << job->errorString();

        beginResetModel();
        m_values.clear();
        endResetModel();
        return;
    }

    const QVariantList revs = dynamic_cast<ReviewBoard::ReviewListRequest *>(job)->reviews();
    beginResetModel();
    m_values.clear();
    for (const QVariant &review : revs) {
        QVariantMap reviewMap = review.toMap();
        QVariantMap repoMap = reviewMap[QStringLiteral("links")].toMap()[QStringLiteral("repository")].toMap();
        if (repoMap[QStringLiteral("title")].toString() == m_repository) {
            m_values += Value{reviewMap[QStringLiteral("summary")], reviewMap[QStringLiteral("id")]};
        }
    }
    endResetModel();
}

QVariant ReviewsListModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.column() != 0 || idx.row() >= m_values.size()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_values[idx.row()].summary;
    case Qt::ToolTipRole:
        return m_values[idx.row()].id;
    }
    return QVariant();
}

int ReviewsListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_values.count();
}

QVariant ReviewsListModel::get(int row, const QByteArray &role)
{
    return index(row, 0).data(roleNames().key(role));
}

#include "moc_rbreviewslistmodel.cpp"
