/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MATCHESSOLVER_H
#define MATCHESSOLVER_H

#include <KJob>

#include "match_p.h"
#include <kpeople/kpeople_export.h>

class QUrl;
class QModelIndex;

namespace KPeople
{
class PersonsModel;

/**
 * @brief This class is responsible for merging different persons into one
 *
 * One of the important tasks of KPeople is putting together different contacts into
 * the same person representation.
 *
 * This class gets a list of matches, usually from @a DuplicatesFinder and makes
 * the needed changes to the database so that everything looks as it should.
 */

class KPEOPLE_EXPORT MatchesSolver : public KJob
{
    Q_OBJECT
public:
    /**
     * Constructs a solver given the @p matches against a @p model.
     *
     * Note: Obviously, the @p model should be the same @p matches refers to.
     */
    explicit MatchesSolver(const QList<Match> &matches, PersonsModel *model, QObject *parent = nullptr);

    /**
     * Start doing some crazy matching!
     *
     * @see KJob::finished()
     */
    void start() override;

private Q_SLOTS:
    void startMatching();

private:
    const QList<Match> m_matches;
    PersonsModel *const m_model;
};
}

#endif // MATCHESSOLVER_H
