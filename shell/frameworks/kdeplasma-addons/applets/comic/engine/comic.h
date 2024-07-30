/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef COMIC_ENGINE_H
#define COMIC_ENGINE_H

// Qt
#include <QIcon>
#include <QSet>
#include <QUrl>
#include <QVariant>

#include "types.h"

class ComicProvider;

/**
 * This class provides the comic strip.
 *
 * The query keys have the following structure:
 *   \<comic_identifier\>:\<suffix\>
 * usually the suffix is the date
 * e.g.
 *   userfriendly:2007-07-19
 * but some other comics uses numerical identifiers, like
 *   xkcd:378
 * if the suffix is empty the latest comic will be returned
 *
 */
class ComicEngine : public QObject
{
    Q_OBJECT

public:
    ComicEngine(QObject *parent);

    QList<ComicProviderInfo> loadProviders();

    void setMaxComicLimit(int maxComicLimit);
    bool requestSource(const QString &identifier);

Q_SIGNALS:
    void requestFinished(const ComicMetaData &data);

private:
    void finished(ComicProvider *);
    void error(ComicProvider *);
    ComicMetaData metaDataFromProvider(ComicProvider *provider);
    QString lastCachedIdentifier(const QString &identifier) const;
    bool isOnline() const;

private:
    bool mEmptySuffix;
    QString mIdentifierError;
    QHash<QString, ComicProvider *> m_jobs;
    QSet<QString> mProviders;
};

#endif
