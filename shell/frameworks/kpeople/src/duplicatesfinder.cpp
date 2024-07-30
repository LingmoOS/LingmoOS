/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "duplicatesfinder_p.h"
#include "personsmodel.h"

#include "kpeople_debug.h"

using namespace KPeople;

DuplicatesFinder::DuplicatesFinder(PersonsModel *model, QObject *parent)
    : KJob(parent)
    , m_model(model)
{
}

void DuplicatesFinder::setSpecificPerson(const QString &personUri)
{
    m_personUri = personUri;
}

void DuplicatesFinder::start()
{
    if (m_personUri.isEmpty()) {
        QMetaObject::invokeMethod(this, "doSearch", Qt::QueuedConnection);
    } else {
        QMetaObject::invokeMethod(this, "doSpecificSearch", Qt::QueuedConnection);
    }
}

// TODO: start providing partial results so that we can start processing matches while it's not done
void DuplicatesFinder::doSearch()
{
    // NOTE: This can probably optimized. I'm just trying to get the semantics right at the moment
    // maybe using nepomuk for the matching would help?

    QList<AbstractContact::Ptr> collectedValues;
    m_matches.clear();

    if (m_model->rowCount() == 0) {
        qCWarning(KPEOPLE_LOG) << "finding duplicates on empty model!";
    }

    for (int i = 0, rows = m_model->rowCount(); i < rows; i++) {
        QModelIndex idx = m_model->index(i, 0);

        // we gather the values
        AbstractContact::Ptr values = idx.data(PersonsModel::PersonVCardRole).value<AbstractContact::Ptr>();

        // we check if it matches
        int j = 0;
        for (const AbstractContact::Ptr &valueToCompare : std::as_const(collectedValues)) {
            QList<Match::MatchReason> matchedRoles = Match::matchAt(values, valueToCompare);

            if (!matchedRoles.isEmpty()) {
                QPersistentModelIndex i2(m_model->index(j, 0));

                m_matches.append(Match(matchedRoles, idx, i2));
            }
            j++;
        }

        // we add our data for comparing later
        collectedValues.append(values);
    }
    emitResult();
}

void DuplicatesFinder::doSpecificSearch()
{
    m_matches.clear();

    QModelIndex idx = m_model->indexForPersonUri(m_personUri);
    AbstractContact::Ptr values = idx.data(PersonsModel::PersonVCardRole).value<AbstractContact::Ptr>();

    for (int i = 0, rows = m_model->rowCount(); i < rows; i++) {
        QModelIndex idx2 = m_model->index(i, 0);

        if (idx2.data(PersonsModel::PersonUriRole) == m_personUri) {
            continue;
        }

        AbstractContact::Ptr values2 = idx2.data(PersonsModel::PersonVCardRole).value<AbstractContact::Ptr>();
        QList<Match::MatchReason> matchedRoles = Match::matchAt(values, values2);
        if (!matchedRoles.isEmpty()) {
            m_matches.append(Match(matchedRoles, idx, idx2));
        }
    }

    emitResult();
}

QList<Match> DuplicatesFinder::results() const
{
    return m_matches;
}

#include "moc_duplicatesfinder_p.cpp"
