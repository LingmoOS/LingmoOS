/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "enclosure.h"

namespace Syndication
{
Enclosure::~Enclosure()
{
}

QString Enclosure::debugInfo() const
{
    QString info = QLatin1String("# Enclosure begin #################\n");

    const QString durl = url();

    if (!durl.isNull()) {
        info += QLatin1String("url: #") + durl + QLatin1String("#\n");
    }

    const QString dtitle = title();

    if (!dtitle.isNull()) {
        info += QLatin1String("title: #") + dtitle + QLatin1String("#\n");
    }

    const QString dtype = type();

    if (!dtype.isNull()) {
        info += QLatin1String("type: #") + dtype + QLatin1String("#\n");
    }

    const int dlength = length();

    if (dlength != 0) {
        info += QLatin1String("length: #") + QString::number(dlength) + QLatin1String("#\n");
    }

    uint dduration = duration();

    if (dduration != 0) {
        int hours = dduration / 3600;
        int minutes = (dduration - hours * 3600) / 60;
        int seconds = dduration - hours * 3600 - minutes * 60;
        info += QStringLiteral("duration: #%1 (%2:%3:%4)#\n").arg(dduration).arg(hours).arg(minutes).arg(seconds);
    }

    info += QLatin1String("# Enclosure end ###################\n");

    return info;
}

} // namespace Syndication
