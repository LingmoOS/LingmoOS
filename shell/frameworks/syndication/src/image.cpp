/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "image.h"

namespace Syndication
{
Image::~Image()
{
}

QString Image::debugInfo() const
{
    QString info = QLatin1String("# Image begin #####################\n");

    QString durl = url();

    if (!durl.isNull()) {
        info += QLatin1String("url: #") + durl + QLatin1String("#\n");
    }

    QString dtitle = title();

    if (!dtitle.isNull()) {
        info += QLatin1String("title: #") + dtitle + QLatin1String("#\n");
    }

    QString dlink = link();

    if (!dlink.isNull()) {
        info += QLatin1String("link: #") + dlink + QLatin1String("#\n");
    }

    QString ddescription = description();

    if (!ddescription.isNull()) {
        info += QLatin1String("description: #") + ddescription + QLatin1String("#\n");
    }

    int dheight = height();
    if (dheight > 0) {
        info += QLatin1String("height: #") + QString::number(height()) + QLatin1String("#\n");
    }

    int dwidth = width();
    if (dwidth > 0) {
        info += QLatin1String("width: #") + QString::number(width()) + QLatin1String("#\n");
    }
    info += QLatin1String("# Image end #######################\n");

    return info;
}

} // namespace Syndication
