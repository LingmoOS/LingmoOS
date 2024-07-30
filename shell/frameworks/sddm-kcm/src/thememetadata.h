/*
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef THEMEMETADATA_H
#define THEMEMETADATA_H

#include <QSharedDataPointer>
#include <QString>

class ThemeMetadataPrivate;

class ThemeMetadata
{
public:
    explicit ThemeMetadata(const QString &id, const QString &path = QString());
    ThemeMetadata(const ThemeMetadata &other);
    ThemeMetadata &operator=(const ThemeMetadata &other);

    ~ThemeMetadata();

    QString path() const;
    QString name() const;
    QString description() const;
    QString author() const;
    QString email() const;
    QString version() const;
    QString website() const;
    QString license() const;
    QString themeapi() const;
    QString screenshot() const;
    QString mainscript() const;
    QString copyright() const;
    QString themeid() const;
    QString configfile() const;
    bool supportsBackground() const;

private:
    void read(const QString &filename);

private:
    QSharedDataPointer<ThemeMetadataPrivate> d;
};
#endif // THEMEMETADATA_H
