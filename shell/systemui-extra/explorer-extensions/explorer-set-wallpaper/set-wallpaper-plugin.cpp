/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019, Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Meihong <hemeihong@kylinos.cn>
 *
 */

#include "set-wallpaper-plugin.h"
#include <file-info.h>
#include <gio/gio.h>

#include <QAction>
#include <QFileInfo>
#include <QTranslator>
#include <QLocale>
#include <QApplication>
#include <QProcess>
#include <QUrl>
#include <QDebug>

#define BACKGROUND_SETTINGS "org.mate.background"
#define PICTRUE "picture-filename"

#define BACKGROUND_SETTINGS_PATH "/org/mate/desktop/background/"

#include <gio/gio.h>

using namespace Peony;

SetWallPaperPlugin::SetWallPaperPlugin(QObject *parent) : QObject (parent)
{
    QTranslator *t = new QTranslator(this);
    QString path = ":/translations/explorer-set-wallpaper-extension_"+QLocale::system().name();
    qDebug()<<"system().name and path:"<<QLocale::system().name() << path;
    qDebug()<<"\n\n\n translate:"<<t->load(path);
    QApplication::installTranslator(t);
}

QList<QAction*> SetWallPaperPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
    QList<QAction*> actions;
    if (types == MenuPluginInterface::DirectoryView || types == MenuPluginInterface::DesktopWindow)
    {
        if (selectionUris.count() == 1 && is_picture_file(selectionUris.first())) {
            if (selectionUris.first().contains("trash:///"))
                return actions;

            g_autoptr (GSettings) background_settings = g_settings_new_with_path(BACKGROUND_SETTINGS, BACKGROUND_SETTINGS_PATH);
            if (background_settings && g_settings_is_writable(background_settings, PICTRUE)) {
                QAction *set_action = new QAction(tr("Set as wallpaper"), nullptr);
                actions<<set_action;
                connect(set_action, &QAction::triggered, [=](){
                    if (QGSettings::isSchemaInstalled(BACKGROUND_SETTINGS))
                    {
                       m_bg_settings = new QGSettings(BACKGROUND_SETTINGS, QByteArray(), this);
                       QUrl url= selectionUris.first();
                       bool success = m_bg_settings->trySet("pictureFilename", url.path());
                       qDebug() << "set as wallpaper result:" <<success <<url.path();
                    }
                });
            }
        }
    }


    return actions;
}

bool SetWallPaperPlugin::is_picture_file(QString file_name)
{
    QFileInfo file(file_name);
    qDebug() << "file_name:" <<file.suffix();
    if (m_picture_type_list.contains(file.suffix()))
        return true;

    return false;
}

