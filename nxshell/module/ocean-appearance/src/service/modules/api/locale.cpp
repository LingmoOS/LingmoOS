// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "locale.h"

#include <stdlib.h>
#include <pthread.h>
#include <QFile>

#define ComponentCodeset 1
#define ComponentTerritory 2
#define ComponentModifier 4

#define MAXLINELEN 256

const char *aliasFile = "/usr/share/locale/locale.alias";
const char charUscore = '_';
const char charDot = '.';
const char charAt = '@';

Locale::Locale()
{
    pthread_mutex_init(&languageNames.mutex, nullptr);

    // init aliases

    QFile file(aliasFile);
    if (file.open(QIODevice::ReadOnly))
    {

        QString line;
        QStringList parts;
        while (!file.atEnd())
        {
            line = file.readLine();

            // 移除行首空行
            line.replace(QRegExp("^ +"),"");
            if(line.front()=='#')
            {
                continue;
            }

            line.replace(QRegExp("\\t$"),"");
            line.replace(QRegExp("\\r$"),"");
            line.replace(QRegExp("\\n$"),"");

            parts = line.split(" ");

            if (parts.size() != 2)
                parts = line.split('\t');

            if (parts.size() == 2) {
                aliases[parts[0]] = parts[1];
            }
        }
        file.close();
    }
}

// wayland environment is useful?
// ExplodeLocale Break an X/Open style locale specification into components
Locale::Components Locale::explodeLocale(QString locale)
{
    Components cmp;
    QStringList parts;
    if (locale.indexOf(charAt) != -1) {
        parts = locale.split(charAt);
        if (parts.size() == 2) {
            cmp.modifier = parts[1];
            locale = parts[0];
            cmp.mask |= ComponentModifier;
        }
    }

    if (locale.indexOf(charDot) != -1) {
        parts = locale.split(charDot);
        if (parts.size() == 2) {
            cmp.codeset = parts[1];
            locale = locale[0];
            cmp.mask |= ComponentCodeset;
        }
    }

    if (locale.indexOf(charUscore) != -1) {
        parts = locale.split(charUscore);
        if (parts.size() == 2) {
            cmp.territory = parts[1];
            locale = parts[0];
            cmp.mask |= ComponentTerritory;
        }
    }

    cmp.language = locale;
    return cmp;
}

QString Locale::guessCategoryValue(QString categoryName)
{
    // The highest priority value is the 'LANGUAGE' environment
    // variable.  This is a GNU extension.
    const char *language = getenv("LANGUAGE");
    if (language)
        return language;

    // Setting of LC_ALL overwrites all other.
    const char *lcAll = getenv("LC_ALL");
    if (lcAll)
        return lcAll;

    // Next comes the name of the desired category.
    const char *name = getenv(categoryName.toLatin1().data());
    if (name)
        return name;

    // Last possibility is the LANG environment variable.
    const char *lang = getenv("LANG");
    if (lang)
        return lang;

    return "C";
}

QString Locale::unaliasLang(QString lang)
{
    if (aliases.find(lang) != aliases.end())
        return aliases[lang];
    else
        return lang;
}

// wayland environment is useful?
/*
 * Compute all interesting variants for a given locale name -
 * by stripping off different components of the value.
 *
 * For simplicity, we assume that the locale is in
 * X/Open format: language[_territory][.codeset][@modifier]
 */
QVector<QString> Locale::getLocaleVariants(const QString &locale)
{
    auto cmp = explodeLocale(locale);
    uint mask = cmp.mask;
    QVector<QString> variants;
    for (uint i = 0; i <= mask; i++) {
        uint j = mask - i;
        //if ((j & ^mask) == 0) {
            QString var(cmp.language);
            if (j & ComponentTerritory)
                var = var + charUscore + cmp.territory;
            if (j & ComponentCodeset)
                var = var + charDot + cmp.codeset;
            if (j & ComponentModifier)
                var = var + charAt + cmp.modifier;

            variants.push_back(var);
        //}
    }

    return variants;
}

QVector<QString> Locale::getLanguageNames()
{
    QVector<QString> names;
    QString value(guessCategoryValue("LC_MESSAGES"));
    if (value.isEmpty()) {
        names.push_back(value);
        return names;
    }

    pthread_mutex_lock(&languageNames.mutex);
    if (languageNames.language != value) {
        languageNames.language = value;
        languageNames.names.clear();
        QStringList langs = value.split(":");
        for (const auto & lang : langs) {
            QVector<QString> localeVariant = getLocaleVariants(unaliasLang(lang));
            for (const auto & var : localeVariant)
                languageNames.names.push_back(var);
        }
        languageNames.names.push_back("C");
    }
    pthread_mutex_unlock(&languageNames.mutex);

    return languageNames.names;
}
