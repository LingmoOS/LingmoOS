/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "feed.h"
#include "category.h"
#include "image.h"
#include "item.h"
#include "person.h"

#include <QList>
#include <QString>

namespace Syndication
{
Feed::~Feed()
{
}

QString Feed::debugInfo() const
{
    QString info = QLatin1String("# Feed begin ######################\n");

    const QString dtitle = title();
    if (!dtitle.isNull()) {
        info += QLatin1String("title: #") + dtitle + QLatin1String("#\n");
    }

    const QString dlink = link();
    if (!dlink.isNull()) {
        info += QLatin1String("link: #") + dlink + QLatin1String("#\n");
    }

    const QString ddescription = description();
    if (!ddescription.isNull()) {
        info += QLatin1String("description: #") + ddescription + QLatin1String("#\n");
    }

    const QString dcopyright = copyright();
    if (!dcopyright.isNull()) {
        info += QLatin1String("copyright: #") + dcopyright + QLatin1String("#\n");
    }

    const QString dlanguage = language();
    if (!dlanguage.isNull()) {
        info += QLatin1String("language: #") + dlanguage + QLatin1String("#\n");
    }

    const QList<PersonPtr> dauthors = authors();
    for (const auto &author : dauthors) {
        info += author->debugInfo();
    }

    const QList<CategoryPtr> dcategories = categories();
    for (const auto &catPtr : dcategories) {
        info += catPtr->debugInfo();
    }

    ImagePtr dimage = image();

    if (!dimage->isNull()) {
        info += dimage->debugInfo();
    }

    ImagePtr dicon = icon();

    if (!dicon->isNull()) {
        info += dicon->debugInfo();
    }

    const QList<ItemPtr> ditems = items();

    for (const auto &item : ditems) {
        info += item->debugInfo();
    }

    info += QLatin1String("# Feed end ########################\n");

    return info;
}

} // namespace Syndication
