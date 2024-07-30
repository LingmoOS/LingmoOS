/*
 * This file is part of the KDE Milou Project
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 *
 */

#pragma once

#include <KConfigGroup>
#include <QAbstractItemModel>
#include <QHash>
#include <QString>

#include <KRunner/QueryMatch>

namespace KRunner
{
class RunnerManager;
}

namespace KRunner
{
class RunnerResultsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit RunnerResultsModel(const KConfigGroup &configGroup, const KConfigGroup &stateConfigGroup, QObject *parent = nullptr);

    QString queryString() const;
    void setQueryString(const QString &queryString, const QString &runner);
    Q_SIGNAL void queryStringChanged(const QString &queryString);

    bool querying() const;
    Q_SIGNAL void queryingChanged();

    /**
     * Clears the model content and resets the runner context, i.e. no new items will appear.
     */
    void clear();

    bool run(const QModelIndex &idx);
    bool runAction(const QModelIndex &idx, int actionNumber);

    int columnCount(const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    KRunner::RunnerManager *runnerManager() const;
    KRunner::QueryMatch fetchMatch(const QModelIndex &idx) const;

    QStringList m_favoriteIds;
Q_SIGNALS:
    void queryStringChangeRequested(const QString &queryString, int pos);

    void matchesChanged();

private:
    void setQuerying(bool querying);

    void onMatchesChanged(const QList<KRunner::QueryMatch> &matches);

    KRunner::RunnerManager *m_manager;

    QString m_queryString;
    bool m_querying = false;

    QString m_prevRunner;

    bool m_hasMatches = false;

    QStringList m_categories;
    QHash<QString /*category*/, QList<KRunner::QueryMatch>> m_matches;
};

} // namespace Milou
