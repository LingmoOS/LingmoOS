// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
