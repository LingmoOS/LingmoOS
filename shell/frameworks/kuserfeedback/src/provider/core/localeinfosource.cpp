/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "localeinfosource.h"

#include <QLocale>
#include <QVariant>

using namespace KUserFeedback;

LocaleInfoSource::LocaleInfoSource()
    : AbstractDataSource(QStringLiteral("locale"))
{
}

QString LocaleInfoSource::description() const
{
    return tr("The current region and language settings.");
}

QVariant LocaleInfoSource::data()
{
    QLocale l;
    QVariantMap m;
    m.insert(QStringLiteral("region"), QLocale::countryToString(l.country()));
    m.insert(QStringLiteral("language"), QLocale::languageToString(l.language()));
    return m;
}

QString LocaleInfoSource::name() const
{
    return tr("Locale information");
}
