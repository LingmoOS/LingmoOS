/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2013 David Edmundson <david@davidedmundson.co.uk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpeople_debug.h"
#include "match_p.h"
#include "matchessolver_p.h"
#include "personmanager_p.h"
#include "personsmodel.h"

#include <QString>

using namespace KPeople;

MatchesSolver::MatchesSolver(const QList<Match> &matches, PersonsModel *model, QObject *parent)
    : KJob(parent)
    , m_matches(matches)
    , m_model(model)
{
}

void MatchesSolver::start()
{
    QMetaObject::invokeMethod(this, "startMatching", Qt::QueuedConnection);
}

void MatchesSolver::startMatching()
{
    // Will contain all the sets to be merged
    QHash<QString, QSet<QString>> jobsData;
    // has a relation of each person, to know where it is
    QHash<QString, QString> destinationResolver;
    for (const Match &m : std::as_const(m_matches)) {
        QString urlA = m.indexA.data(PersonsModel::PersonUriRole).toString();
        QString urlB = m.indexB.data(PersonsModel::PersonUriRole).toString();
        Q_ASSERT(urlA != urlB);

        const bool inA = destinationResolver.contains(urlA);
        const bool inB = destinationResolver.contains(urlB);
        if (inA && inB) {
            // Both are in already, so we have to merge both sets
            destinationResolver[urlB] = urlA;
            jobsData[urlA] = jobsData.take(urlB);

            // we've put all items pointed to by urlA, to the B set
            // now we re-assign anything pointing to B as pointing to A
            // because they are the same
            const auto keys = destinationResolver.keys(urlB);
            auto it = keys.constBegin();
            const auto end = keys.constEnd();
            for (; it != end; ++it) {
                destinationResolver[*it] = urlA;
            }
        } else {
            // if A is in but not B, we want B wherever A is
            if (inA) {
                qSwap(urlB, urlA);
            }
            // in case B is anywhere, add A to that set, otherwise just insert B
            const QString mergeUrl = destinationResolver.value(urlB, urlB);

            QSet<QString> &jobs = jobsData[mergeUrl];
            jobs.insert(urlB);
            jobs.insert(urlA);

            // remember where urlA and urlB are
            Q_ASSERT(urlA != mergeUrl);
            destinationResolver.insert(urlA, mergeUrl);
            if (urlB != mergeUrl) {
                destinationResolver.insert(urlB, mergeUrl);
            }
        }
    }

    for (const QSet<QString> &uris : std::as_const(jobsData)) {
        if (PersonManager::instance()->mergeContacts(uris.values()).isEmpty()) {
            qCWarning(KPEOPLE_LOG) << "error: failing to merge contacts: " << uris;
        }
    }
    emitResult();
}

#include "moc_matchessolver_p.cpp"
