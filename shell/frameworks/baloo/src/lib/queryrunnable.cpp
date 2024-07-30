/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "queryrunnable.h"
#include <QAtomicInt>

using namespace Baloo;

class BALOO_CORE_NO_EXPORT QueryRunnable::Private {
public:
    Query m_query;
    QAtomicInt m_stop;

    bool stopRequested() const {
        return m_stop.loadRelaxed();
    }

};

QueryRunnable::QueryRunnable(const Query& query, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->m_query = query;
    d->m_stop = false;
}

QueryRunnable::~QueryRunnable() = default;

void QueryRunnable::stop()
{
    d->m_stop.storeRelaxed(true);
}

void QueryRunnable::run()
{
    ResultIterator it = d->m_query.exec();
    while (!d->stopRequested() && it.next()) {
        Q_EMIT queryResult(this, it.filePath());
    }

    Q_EMIT finished(this);
}

#include "moc_queryrunnable.cpp"
