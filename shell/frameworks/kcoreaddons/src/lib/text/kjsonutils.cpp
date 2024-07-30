/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kjsonutils.h"

#include <QJsonObject>

QJsonValue KJsonUtils::readTranslatedValue(const QJsonObject &jo, const QString &key, const QJsonValue &defaultValue)
{
    QString languageWithCountry = QLocale().name();
    auto it = jo.constFind(key + QLatin1Char('[') + languageWithCountry + QLatin1Char(']'));
    if (it != jo.constEnd()) {
        return it.value();
    }
    const QStringView language = QStringView(languageWithCountry).mid(0, languageWithCountry.indexOf(QLatin1Char('_')));
    it = jo.constFind(key + QLatin1Char('[') + language + QLatin1Char(']'));
    if (it != jo.constEnd()) {
        return it.value();
    }
    // no translated value found -> check key
    it = jo.constFind(key);
    if (it != jo.constEnd()) {
        return jo.value(key);
    }
    return defaultValue;
}

QString KJsonUtils::readTranslatedString(const QJsonObject &jo, const QString &key, const QString &defaultValue)
{
    return KJsonUtils::readTranslatedValue(jo, key, defaultValue).toString(defaultValue);
}
