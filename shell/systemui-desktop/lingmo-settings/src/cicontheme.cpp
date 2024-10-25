/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cicontheme.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>

CIconTheme::CIconTheme(QObject *parent)
    : QObject(parent)
{

}

QStringList CIconTheme::list()
{
    QStringList list;
    QStringList iconsPath;
    iconsPath += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("icons"), QStandardPaths::LocateDirectory);
    iconsPath += QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("pixmaps"), QStandardPaths::LocateDirectory);

    for (const QString &path : qAsConst(iconsPath)) {
        QDir dir(path);
        QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &dir : dirList) {
            const QString &itemPath = path + "/" + dir;
            if (itemPath.startsWith("default.")) {
                continue;
            }

            if (!QFileInfo::exists(itemPath + "/index.theme")) {
                continue;
            }

            list.append(path + "/" + dir);
        }
    }

    return list;
}
