/*
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef COMIC_ENGINE_TYPES
#define COMIC_ENGINE_TYPES

#include <QIcon>
#include <QString>
#include <QUrl>

/**
 * Describes the type of how this comic provider
 * references the previous or next comic strip.
 */
enum class IdentifierType {
    DateIdentifier = 0, ///< References by date
    NumberIdentifier, ///< References by numerical identifier
    StringIdentifier, ///< References by arbitrary string
};

inline IdentifierType stringToIdentifierType(const QString type)
{
    if (type == QLatin1String("Date")) {
        return IdentifierType::DateIdentifier;
    } else if (type == QLatin1String("Number")) {
        return IdentifierType::NumberIdentifier;
    } else if (type == QLatin1String("String")) {
        return IdentifierType::StringIdentifier;
    }
    return IdentifierType::StringIdentifier;
}

struct ComicProviderInfo {
    QString pluginId;
    QString name;
    QString icon;
};

struct ComicMetaData {
    QString stripTitle;
    QUrl imageUrl;
    QImage image;
    QUrl websiteUrl;
    QUrl shopUrl;
    QString firstStripIdentifier;
    QString previousStripIdentifier;
    QString nextIdentifier;
    QString previousIdentifier;
    QString author;
    QString comicAuthor;
    QString additionalText;
    QString identifier;
    IdentifierType identifierType;
    bool isLeftToRight = false;
    bool isTopToBottom = false;
    QString lastCachedStripIdentifier;
    QString providerName;
    bool error = false;
    bool errorAutomaticallyFixable = false;
};

#endif
