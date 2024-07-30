/*
    This file is part of the syndication library
    SPDX-FileCopyrightText: 2006 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "category.h"

namespace Syndication
{
Category::~Category()
{
}

QString Category::debugInfo() const
{
    QString info = QLatin1String("# Category begin ##################\n");

    QString dterm = term();

    if (!dterm.isNull()) {
        info += QLatin1String("term: #") + dterm + QLatin1String("#\n");
    }

    QString dscheme = scheme();

    if (!dscheme.isNull()) {
        info += QLatin1String("scheme: #") + dscheme + QLatin1String("#\n");
    }

    QString dlabel = label();

    if (!dlabel.isNull()) {
        info += QLatin1String("label: #") + dlabel + QLatin1String("#\n");
    }

    info += QLatin1String("# Category end ####################\n");

    return info;
}

} // namespace Syndication
