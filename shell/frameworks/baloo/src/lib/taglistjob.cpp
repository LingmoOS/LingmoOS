/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "taglistjob.h"
#include "global.h"
#include "database.h"
#include "transaction.h"

#include <QStringList>

using namespace Baloo;

class BALOO_CORE_NO_EXPORT TagListJob::Private {
public:
    QStringList tags;
};

TagListJob::TagListJob(QObject* parent)
    : KJob(parent)
    , d(new Private)
{
}

TagListJob::~TagListJob() = default;

void TagListJob::start()
{
    Database *db = globalDatabaseInstance();

    if (!db->open(Database::ReadOnlyDatabase)) {
        // if we have no index, we have no tags
        if (!db->isAvailable()) {
            emitResult();
            return;
        }

        setError(UserDefinedError);
        setErrorText(QStringLiteral("Failed to open the database"));
        emitResult();
        return;
    }

    QVector<QByteArray> tagList;
    {
        Transaction tr(db, Transaction::ReadOnly);
        tagList = tr.fetchTermsStartingWith("TAG-");
    }
    d->tags.reserve(tagList.size());
    for (const QByteArray& ba : std::as_const(tagList)) {
        d->tags << QString::fromUtf8(ba.mid(4));
    }

    emitResult();
}

QStringList TagListJob::tags()
{
    return d->tags;
}

#include "moc_taglistjob.cpp"
