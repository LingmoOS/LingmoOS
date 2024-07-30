/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "thememetadata.h"

#include <QFile>
#include <QSharedData>
#include <QSharedPointer>

#include <KConfigGroup>
#include <KDesktopFile>

class ThemeMetadataPrivate : public QSharedData
{
public:
    QString themeid;
    QString name;
    QString description;
    QString author;
    QString email;
    QString version;
    QString website;
    QString license;
    QString themeapi;
    QString mainscript;
    QString screenshot;
    QString copyright;
    QString path;
    QString configfile;
    bool supportsBackground;
};

ThemeMetadata::ThemeMetadata(const QString &id, const QString &path)
    : d(new ThemeMetadataPrivate)
{
    d->path = path + QLatin1Char('/');
    read(d->path + QStringLiteral("metadata.desktop"));
    d->themeid = id;
}

ThemeMetadata::ThemeMetadata(const ThemeMetadata &other)
    : d(other.d)
{
}

ThemeMetadata &ThemeMetadata::operator=(const ThemeMetadata &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

ThemeMetadata::~ThemeMetadata()
{
}

void ThemeMetadata::read(const QString &filename)
{
    if (filename.isEmpty()) {
        return;
    }

    QSharedPointer<KConfig> configFile = QSharedPointer<KConfig>(new KConfig(filename, KConfig::SimpleConfig));

    KConfigGroup config = configFile->group(QStringLiteral("SddmGreeterTheme"));

    // d->themeid = config.readEntry("Theme-Id");
    d->name = config.readEntry("Name");
    d->description = config.readEntry("Description");
    d->author = config.readEntry("Author");
    d->email = config.readEntry("Email");
    d->version = config.readEntry("Version");
    d->website = config.readEntry("Website");
    d->license = config.readEntry("License");
    d->themeapi = config.readEntry("Theme-API");
    d->mainscript = config.readEntry("MainScript");
    d->screenshot = config.readEntry("Screenshot");
    d->copyright = config.readEntry("Copyright");
    d->configfile = config.readEntry("ConfigFile");

    d->supportsBackground = QFile::exists(d->path + d->configfile);
}

QString ThemeMetadata::path() const
{
    return d->path;
}

QString ThemeMetadata::themeid() const
{
    return d->themeid;
}

QString ThemeMetadata::name() const
{
    return d->name;
}

QString ThemeMetadata::description() const
{
    return d->description;
}

QString ThemeMetadata::author() const
{
    return d->author;
}

QString ThemeMetadata::version() const
{
    return d->version;
}

QString ThemeMetadata::email() const
{
    return d->email;
}

QString ThemeMetadata::website() const
{
    return d->website;
}

QString ThemeMetadata::license() const
{
    return d->license;
}

QString ThemeMetadata::themeapi() const
{
    return d->themeapi;
}

QString ThemeMetadata::mainscript() const
{
    return d->mainscript;
}

QString ThemeMetadata::screenshot() const
{
    return d->screenshot;
}

QString ThemeMetadata::copyright() const
{
    return d->copyright;
}

QString ThemeMetadata::configfile() const
{
    return d->configfile;
}

bool ThemeMetadata::supportsBackground() const
{
    return d->supportsBackground;
}
