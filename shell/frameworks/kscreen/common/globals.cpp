/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "globals.h"

#include <QStandardPaths>
#include <QStringBuilder>

namespace Globals
{

QString dirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) % QStringLiteral("/kscreen/");
}

QString findFile(const QString &filePath)
{
    return QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kscreen/") % filePath);
}
}
