/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include "cachedprovider.h"
#include "comic_debug.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

const int CachedProvider::CACHE_DEFAULT = 20;

static QString identifierToPath(const QString &identifier)
{
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/plasma_engine_comic/");

    return dataDir + QString::fromLatin1(QUrl::toPercentEncoding(identifier));
}

CachedProvider::CachedProvider(QObject *parent, const KPluginMetaData &data, IdentifierType type, const QString &identifier)
    : ComicProvider(parent, data, type, identifier)
{
    QTimer::singleShot(0, this, &CachedProvider::triggerFinished);
}

CachedProvider::~CachedProvider()
{
}

IdentifierType CachedProvider::identifierType() const
{
    return IdentifierType::StringIdentifier;
}

QImage CachedProvider::image() const
{
    if (!QFile::exists(identifierToPath(requestedString()))) {
        return QImage();
    }

    QImage img;
    img.load(identifierToPath(requestedString()), "PNG");

    return img;
}

QString CachedProvider::identifier() const
{
    if (requestedString().lastIndexOf(QLatin1Char(':')) + 1 == requestedString().size()) {
        return requestedString() + lastCachedStripIdentifier();
    }
    return requestedString();
}

QString CachedProvider::nextIdentifier() const
{
    QSettings settings(identifierToPath(requestedString()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("nextIdentifier"), QString()).toString();
}

QString CachedProvider::previousIdentifier() const
{
    QSettings settings(identifierToPath(requestedString()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("previousIdentifier"), QString()).toString();
}

QString CachedProvider::firstStripIdentifier() const
{
    QSettings settings(identifierToPath(requestedComicName()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("firstStripIdentifier"), QString()).toString();
}

QString CachedProvider::lastCachedStripIdentifier() const
{
    QSettings settings(identifierToPath(requestedComicName()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("lastCachedStripIdentifier"), QString()).toString();
}

QString CachedProvider::comicAuthor() const
{
    QSettings settings(identifierToPath(requestedString()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("comicAuthor"), QString()).toString();
}

QString CachedProvider::stripTitle() const
{
    QSettings settings(identifierToPath(requestedString()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("stripTitle"), QString()).toString();
}

QString CachedProvider::additionalText() const
{
    QSettings settings(identifierToPath(requestedString()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("additionalText"), QString()).toString();
}

QString CachedProvider::name() const
{
    QSettings settings(identifierToPath(requestedComicName()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("title"), QString()).toString();
}

void CachedProvider::triggerFinished()
{
    Q_EMIT finished(this);
}

bool CachedProvider::isCached(const QString &identifier)
{
    return QFile::exists(identifierToPath(identifier));
}

bool CachedProvider::storeInCache(const QString &identifier, const QImage &comic, const ComicMetaData &data)
{
    const QString path = identifierToPath(identifier);

    int index = identifier.indexOf(QLatin1Char(':'));
    const QString comicName = identifier.mid(0, index);
    const QString pathMain = identifierToPath(comicName);
    const QString dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/plasma_engine_comic/");

    QSettings settingsMain(pathMain + QLatin1String(".conf"), QSettings::IniFormat);
    settingsMain.setValue(QStringLiteral("firstStripIdentifier"), data.firstStripIdentifier);
    settingsMain.setValue(QStringLiteral("title"), data.providerName);
    settingsMain.setValue(QStringLiteral("lastCachedStripIdentifier"), data.lastCachedStripIdentifier);
    settingsMain.setValue(QStringLiteral("shopUrl"), data.shopUrl);
    settingsMain.setValue(QStringLiteral("isLeftToRight"), data.isLeftToRight);
    settingsMain.setValue(QStringLiteral("isTopToBottom"), data.isTopToBottom);

    QSettings settings(path + QLatin1String(".conf"), QSettings::IniFormat);
    settings.setValue(QStringLiteral("additionalText"), data.additionalText);
    settings.setValue(QStringLiteral("comicAuthor"), data.comicAuthor);
    settings.setValue(QStringLiteral("imageUrl"), data.imageUrl);
    settings.setValue(QStringLiteral("nextIdentifier"), data.nextIdentifier);
    settings.setValue(QStringLiteral("previousIdentifier"), data.previousIdentifier);
    settings.setValue(QStringLiteral("websiteUrl"), data.websiteUrl);

    QStringList comics;
    if (settingsMain.contains(QLatin1String("comics"))) {
        comics = settingsMain.value(QLatin1String("comics"), QStringList()).toStringList();
    } else {
        // existing strips haven't been stored in the conf-file yet, do that now, oldest first, newest last
        QDir dir(dirPath);
        comics = dir.entryList(QStringList() << QString::fromLatin1(QUrl::toPercentEncoding(comicName + QLatin1Char(':'))) + QLatin1Char('*'),
                               QDir::Files,
                               QDir::Time | QDir::Reversed);
        QStringList::iterator it = comics.begin();
        while (it != comics.end()) {
            // only count images, not the conf files
            if ((*it).endsWith(QLatin1String(".conf"))) {
                it = comics.erase(it);
            } else {
                ++it;
            }
        }
    }
    comics.append(QString::fromLatin1(QUrl::toPercentEncoding(identifier)));

    const int limit = CachedProvider::maxComicLimit();
    // limit is on
    if (limit > 0) {
        qCDebug(PLASMA_COMIC) << QLatin1String("MaxComicLimit on.");
        int comicsToRemove = comics.count() - limit;
        QStringList::iterator it = comics.begin();
        while (comicsToRemove > 0 && it != comics.end()) {
            qCDebug(PLASMA_COMIC) << QLatin1String("Remove file") << (dirPath + (*it));
            QFile::remove(dirPath + (*it));
            QFile::remove(dirPath + (*it) + QLatin1String(".conf"));
            it = comics.erase(it);
            --comicsToRemove;
        }
    }
    settingsMain.setValue(QLatin1String("comics"), comics);

    return comic.save(path, "PNG");
}

QUrl CachedProvider::websiteUrl() const
{
    QSettings settings(identifierToPath(requestedString()) + QLatin1String(".conf"), QSettings::IniFormat);
    return QUrl(settings.value(QLatin1String("websiteUrl")).toUrl());
}

QUrl CachedProvider::imageUrl() const
{
    QSettings settings(identifierToPath(requestedString()) + QLatin1String(".conf"), QSettings::IniFormat);
    return QUrl(settings.value(QLatin1String("imageUrl")).toUrl());
}

QUrl CachedProvider::shopUrl() const
{
    QSettings settings(identifierToPath(requestedComicName()) + QLatin1String(".conf"), QSettings::IniFormat);
    return QUrl(settings.value(QLatin1String("shopUrl")).toUrl());
}

bool CachedProvider::isLeftToRight() const
{
    QSettings settings(identifierToPath(requestedComicName()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("isLeftToRight"), true).toBool();
}

bool CachedProvider::isTopToBottom() const
{
    QSettings settings(identifierToPath(requestedComicName()) + QLatin1String(".conf"), QSettings::IniFormat);
    return settings.value(QLatin1String("isTopToBottom"), true).toBool();
}

int CachedProvider::maxComicLimit()
{
    QSettings settings(identifierToPath(QLatin1String("comic_settings.conf")), QSettings::IniFormat);
    return qMax(settings.value(QLatin1String("maxComics"), CACHE_DEFAULT).toInt(), 0); // old value was -1, thus use qMax
}

void CachedProvider::setMaxComicLimit(int limit)
{
    if (limit < 0) {
        qCDebug(PLASMA_COMIC) << "Wrong limit, setting to default.";
        limit = CACHE_DEFAULT;
    }
    QSettings settings(identifierToPath(QLatin1String("comic_settings.conf")), QSettings::IniFormat);
    settings.setValue(QLatin1String("maxComics"), limit);
}
