/*
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef DUPLICATESFINDER_H
#define DUPLICATESFINDER_H

#include "match_p.h"
#include <KJob>
#include <kpeople/kpeople_export.h>

namespace KContacts
{
class Addressee;
}

namespace KPeople
{
class PersonsModel;
class KPEOPLE_EXPORT DuplicatesFinder : public KJob
{
    Q_OBJECT
public:
    explicit DuplicatesFinder(PersonsModel *model, QObject *parent = nullptr);

    /** Specifies a @p personUri id to look contacts for, in contrast to the whole contact list. */
    void setSpecificPerson(const QString &personUri);

    /** Returns the match results. */
    QList<Match> results() const;

public Q_SLOTS:
    void start() override;

private Q_SLOTS:
    void doSearch();
    void doSpecificSearch();

private:
    PersonsModel *const m_model;
    QList<Match> m_matches;
    QString m_personUri;
};
}

#endif // DUPLICATESFINDER_H
