/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESOURCEINFO_TABLE_H
#define RESOURCEINFO_TABLE_H

#include <QString>

#include "common.h"

namespace ResourceInfo
{
struct Item {
    QString targettedResource;
    QString title;
    QString mimetype;

    const QString &primaryKey() const
    {
        return targettedResource;
    }
};

DECL_COLUMN(QString, targettedResource)
DECL_COLUMN(QString, title)
DECL_COLUMN(QString, mimetype)

}

#endif // RESOURCEINFO_TABLE_H
