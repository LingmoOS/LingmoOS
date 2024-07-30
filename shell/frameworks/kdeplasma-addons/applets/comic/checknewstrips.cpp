/*
 *   SPDX-FileCopyrightText: 2011 Matthias Fuchs <mat69@gmx.net>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "checknewstrips.h"

#include <QTimer>

CheckNewStrips::CheckNewStrips(const QStringList &identifiers, ComicEngine *engine, int minutes, QObject *parent)
    : QObject(parent)
    , mMinutes(minutes)
    , mIndex(0)
    , mEngine(engine)
    , mIdentifiers(identifiers)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(minutes * 60 * 1000);
    connect(timer, &QTimer::timeout, this, &CheckNewStrips::start);
    timer->start();

    // start at once, that way the user does not have to wait for minutes to get the initial result
    start();
    connect(mEngine, &ComicEngine::requestFinished, this, &CheckNewStrips::dataUpdated);
}

void CheckNewStrips::dataUpdated(const ComicMetaData &data)
{
    const QString source = data.identifier;
    QString lastIdentifierSuffix;

    if (!data.error) {
        lastIdentifierSuffix = data.identifier;
        lastIdentifierSuffix.remove(source);
    }

    if (!lastIdentifierSuffix.isEmpty()) {
        QString temp = source;
        temp.remove(QLatin1Char(':'));
        Q_EMIT lastStrip(mIndex, temp, lastIdentifierSuffix);
    }
    ++mIndex;

    if (mIndex < mIdentifiers.count()) {
        const QString newSource = mIdentifiers[mIndex] + QLatin1Char(':');
        mEngine->requestSource(newSource);
    } else {
        mIndex = 0;
    }
}

void CheckNewStrips::start()
{
    // already running, do nothing
    if (mIndex) {
        return;
    }

    if (mIndex < mIdentifiers.count()) {
        const QString newSource = mIdentifiers[mIndex] + QLatin1Char(':');
        mEngine->requestSource(newSource);
    }
}
