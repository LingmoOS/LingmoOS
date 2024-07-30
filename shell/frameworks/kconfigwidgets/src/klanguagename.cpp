/*
    SPDX-FileCopyrightText: 1999-2003 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2007 David Jarvie <software@astrojar.org.uk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "klanguagename.h"

#include <KConfig>
#include <KConfigGroup>

#include <QDir>
#include <QLocale>

QString KLanguageName::nameForCode(const QString &code)
{
    const QStringList parts = QLocale().name().split(QLatin1Char('_'));
    return nameForCodeInLocale(code, parts.at(0));
}

static std::tuple<QString, QString> namesFromEntryFile(const QString &realCode, const QString &realOutputCode)
{
    const QString entryFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                     QStringLiteral("locale") + QLatin1Char('/') + realCode + QStringLiteral("/kf6_entry.desktop"));

    if (!entryFile.isEmpty()) {
        KConfig entry(entryFile, KConfig::SimpleConfig);
        entry.setLocale(realOutputCode);
        const KConfigGroup group(&entry, QStringLiteral("KCM Locale"));
        const QString name = group.readEntry("Name");

        entry.setLocale(QStringLiteral("en_US"));
        const QString englishName = group.readEntry("Name");
        return std::make_tuple(name, englishName);
    }
    return {};
}

QString KLanguageName::nameForCodeInLocale(const QString &code, const QString &outputCode)
{
    const QString realCode = code == QLatin1String("en") ? QStringLiteral("en_US") : code;
    const QString realOutputCode = outputCode == QLatin1String("en") ? QStringLiteral("en_US") : outputCode;

    const std::tuple<QString, QString> nameAndEnglishName = namesFromEntryFile(realCode, realOutputCode);
    const QString name = std::get<0>(nameAndEnglishName);
    const QString englishName = std::get<1>(nameAndEnglishName);

    if (!name.isEmpty()) {
        // KConfig doesn't have a way to say it didn't find the entry in
        // realOutputCode. When it doesn't find it in the locale you ask for, it just returns the english version
        // so we compare the returned name against the english version, if they are different we return it, if they
        // are equal we defer to QLocale (with a final fallback to name/englishName if QLocale doesn't know about it)
        if (name != englishName || realOutputCode == QLatin1String("en_US")) {
            return name;
        }
    }

    const QLocale locale(realCode);
    if (locale != QLocale::c()) {
        if (realCode == realOutputCode) {
            return locale.nativeLanguageName();
        }
        return QLocale::languageToString(locale.language());
    }

    // We get here if QLocale doesn't know about realCode (at the time of writing this happens for crh, csb, hne, mai) and name and englishName are the same.
    // So what we do here is return name, which can be either empty if the KDE side doesn't know about the code, or otherwise will be the name/englishName
    return name;
}

QStringList KLanguageName::allLanguageCodes()
{
    QStringList systemLangList;
    const QStringList localeDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("locale"), QStandardPaths::LocateDirectory);
    for (const QString &localeDir : localeDirs) {
        const QStringList entries = QDir(localeDir).entryList(QDir::Dirs);
        auto languageExists = [&localeDir](const QString &language) {
            return QFile::exists(localeDir + QLatin1Char('/') + language + QLatin1String("/kf6_entry.desktop"));
        };
        std::copy_if(entries.begin(), entries.end(), std::back_inserter(systemLangList), languageExists);
    }
    if (localeDirs.count() > 1) {
        systemLangList.removeDuplicates();
    }
    return systemLangList;
}
