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

#include "createfolderdialog.h"
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>

#include <KIO/MkdirJob>

CreateFolderDialog::CreateFolderDialog(QObject *parent)
    : QObject(parent)
{

}

void CreateFolderDialog::setPath(const QString &path)
{
    m_path = path;
}

void CreateFolderDialog::show()
{
    QQmlApplicationEngine *engine = new QQmlApplicationEngine;
    engine->rootContext()->setContextProperty("main", this);
    engine->load(QUrl("qrc:/qml/Dialogs/CreateFolderDialog.qml"));
}

void CreateFolderDialog::newFolder(const QString &folderName)
{
    if (m_path.isEmpty() || folderName.isEmpty())
        return;

    auto job = KIO::mkdir(QUrl(m_path + "/" + folderName));
    job->start();
}
