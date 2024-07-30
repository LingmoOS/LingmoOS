/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#ifndef CACHEDPROVIDER_H
#define CACHEDPROVIDER_H

#include "comicprovider.h"
#include "types.h"

#include <QHash>

/**
 * This class provides comics from the local cache.
 */
class CachedProvider : public ComicProvider
{
    Q_OBJECT

public:
    /**
     * Creates a new cached provider.
     *
     * @param parent The parent object.
     * param args The arguments.
     */
    explicit CachedProvider(QObject *parent, const KPluginMetaData &data, IdentifierType type, const QString &identifier);

    /**
     * Destroys the cached provider.
     */
    ~CachedProvider() override;

    /**
     * Returns the identifier type.
     *
     * Is always StringIdentifier here.
     */
    IdentifierType identifierType() const override;

    /**
     * Returns the requested image.
     *
     * Note: This method returns only a valid image after the
     *       finished() signal has been emitted.
     */
    QImage image() const override;

    /**
     * Returns the identifier of the comic request (name + date).
     */
    QString identifier() const override;

    /**
     * Returns the identifier suffix of the next comic.
     */
    QString nextIdentifier() const override;

    /**
     * Returns the identifier suffix of the previous comic.
     */
    QString previousIdentifier() const override;

    /**
     * Returns the identifier of the first strip.
     */
    QString firstStripIdentifier() const override;

    /**
     * Returns the identifier of the last cached strip.
     */
    QString lastCachedStripIdentifier() const;

    /**
     * Returns the title of the strip.
     */
    QString stripTitle() const override;

    /**
     * Returns the author of the comic.
     */
    QString comicAuthor() const override;

    /**
     * Returns additionalText of the comic.
     */
    QString additionalText() const override;

    /**
     * Returns the name for the comic
     */
    QString name() const override;

    /**
     * Returns whether the comic is leftToRight or not
     */
    bool isLeftToRight() const override;

    /**
     * Returns whether the comic is topToBottom or not
     */
    bool isTopToBottom() const override;

    /**
     * Returns whether a comic with the given @p identifier is cached.
     */
    static bool isCached(const QString &identifier);

    /**
     * Map of keys and values to store in the config file for an individual identifier
     */
    typedef QHash<QString, QString> Settings;

    /**
     * Stores the given @p comic with the given @p identifier in the cache.
     */
    static bool storeInCache(const QString &identifier, const QImage &comic, const ComicMetaData &info);

    /**
     * Returns the website of the comic.
     */
    QUrl websiteUrl() const override;

    QUrl imageUrl() const override;

    /**
     * Returns the shop website of the comic.
     */
    QUrl shopUrl() const override;

    /**
     * Returns the maximum number of cached strips per comic, -1 means that there is no limit
     * @note defaulte is -1
     */
    static int maxComicLimit();

    /**
     * Sets the maximum number of cached strips per comic, -1 means that there is no limit
     */
    static void setMaxComicLimit(int limit);

private Q_SLOTS:
    void triggerFinished();

private:
    static const int CACHE_DEFAULT;
};

#endif
