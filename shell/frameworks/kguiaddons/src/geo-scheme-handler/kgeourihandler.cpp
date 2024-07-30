/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kgeourihandler_p.h"

#include <QUrl>
#include <QUrlQuery>

void KGeoUriHandler::setCoordinateTemplate(const QString &coordTmpl)
{
    m_coordTmpl = coordTmpl;
}

void KGeoUriHandler::setQueryTemplate(const QString &queryTmpl)
{
    m_queryTmpl = queryTmpl;
}

void KGeoUriHandler::setFallbackUrl(const QString &fallbackUrl)
{
    m_fallbackUrl = fallbackUrl;
}

static bool isValidCoordinate(double c, double limit)
{
    return c != 0.0 && c >= -limit && c <= limit;
}

QString KGeoUriHandler::handleUri(const QUrl &geoUri)
{
    const auto pathElems = geoUri.path().split(QLatin1Char(';'));
    const auto coordElems = pathElems.isEmpty() ? QStringList() : pathElems.at(0).split(QLatin1Char(','));

    const auto lat = coordElems.size() < 2 ? 0.0 : coordElems.at(0).toDouble();
    const auto lon = coordElems.size() < 2 ? 0.0 : coordElems.at(1).toDouble();

    const auto geoQuery = QUrlQuery(geoUri.query());
    const auto query = geoQuery.queryItemValue(QStringLiteral("q"));

    bool zoomValid = false;
    int zoom = geoQuery.queryItemValue(QStringLiteral("z")).toInt(&zoomValid);
    if (!zoomValid || zoom < 0 || zoom > 21) {
        zoom = 18;
    }

    // unsupported coordinate reference system
    if (!pathElems.isEmpty() && std::any_of(pathElems.begin() + 1, pathElems.end(), [](const auto &elem) {
            return elem.startsWith(QLatin1String("crs="), Qt::CaseInsensitive) && !elem.endsWith(QLatin1String("=wgs84"), Qt::CaseInsensitive);
        })) {
        return m_fallbackUrl;
    }

    QString tmpl;
    if (!query.isEmpty()) {
        tmpl = m_queryTmpl;
    } else if (isValidCoordinate(lat, 90.0) && isValidCoordinate(lon, 180.0)) {
        tmpl = m_coordTmpl;
    } else {
        return m_fallbackUrl;
    }

    tmpl.replace(QLatin1String("<LAT>"), QString::number(lat));
    tmpl.replace(QLatin1String("<LON>"), QString::number(lon));
    tmpl.replace(QLatin1String("<Q>"), query);
    tmpl.replace(QLatin1String("<Z>"), QString::number(zoom));
    return tmpl;
}
