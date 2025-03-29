/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#include "fm.h"
#include <QDir>
#include <QUrl>
#include <QStandardPaths>

#include <KIO/EmptyTrashJob>

Fm::Fm(QObject *parent) : QObject(parent)
{

}

QString Fm::rootPath()
{
    return QDir::rootPath();
}

void Fm::emptyTrash()
{
    KIO::Job *job = KIO::emptyTrash();
    job->start();
}

bool Fm::isFixedFolder(const QUrl &folderUrl)
{
    const QString folder = folderUrl.toLocalFile();

    return folder == QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() ||
           folder == QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() ||
           folder == QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first() ||
           folder == QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first() ||
           folder == QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first() ||
           folder == QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first() ||
           folder == QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first();
}
