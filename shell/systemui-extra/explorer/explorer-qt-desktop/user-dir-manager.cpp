/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "user-dir-manager.h"
#include "file-operation-manager.h"
#include "file-rename-operation.h"
#include "global-settings.h"
#include "file-info.h"
#include "file-item.h"
#include <stdio.h>
#include <pwd.h>
#include <unistd.h>
#include <gio/gio.h>
#include <QDir>
#include <QStandardPaths>
#include <QTimer>
#include <QSettings>
#include <QTextCodec>
#include <QFile>

using namespace Peony;
UserdirManager::UserdirManager(QObject *parent) : QObject(parent)
{
    struct passwd *pwd;
    pwd=getpwuid(getuid());
    auto userName = pwd->pw_name;
    m_user_name = QString(userName);

    m_settings = new QSettings("org.lingmo", "explorer-qt-preferences", this);
    m_do_not_thumbnail = m_settings->value(FORBID_THUMBNAIL_IN_VIEW).toBool();
    m_allow_parallel = m_settings->value(ALLOW_FILE_OP_PARALLEL).toBool();
    m_showTrashDialog = m_settings->value(SHOW_TRASH_DIALOG).toBool();
    m_user_dir_watcher = new QFileSystemWatcher(this);

    m_user_path = g_get_home_dir();
    QString path0 = QString(m_user_path +"/.config/user-dirs.dirs");
    QString path1 = QString(m_user_path +"/.config/org.lingmo/explorer-qt-preferences.conf");

    if(!QFile(path0).exists())
    {
        QTimer *timer = new QTimer;
        connect(timer,&QTimer::timeout,[=](){
            if(QFile(path0).exists())
            {
                qWarning()<<"m_times............."<<m_times;
                getUserdir();
                m_user_dir_watcher->addPath(path0);
                timer->stop();
            }
            else
            {
                if(--m_times==0)
                {
                    timer->stop();
                    qWarning()<<"stop finding user-dirs.dirs";
                }
            }
        });
        timer->start(1000);
    }
    else
    {
        m_user_dir_watcher->addPath(path0);
        getUserdir();
    }

    if(!QFile(path1).exists())
    {
        GlobalSettings::getInstance();
    }
    m_user_dir_watcher->addPath(path1);

    connect(m_user_dir_watcher, &QFileSystemWatcher::fileChanged, [=](const QString &uri){
        //user-dirs.dirs
        if(uri == path0)
        {
            getUserdir();
            Q_EMIT desktopDirChanged();
//                moveFile();
        }
        //explorer-qt-preferences  only thumbnail setting for now.
        else if(uri == path1)
        {
            m_settings = new QSettings("org.lingmo", "explorer-qt-preferences", this);
            if(m_do_not_thumbnail != m_settings->value(FORBID_THUMBNAIL_IN_VIEW).toBool())
            {
                m_do_not_thumbnail = m_settings->value(FORBID_THUMBNAIL_IN_VIEW).toBool();
                Q_EMIT thumbnailSetingChange();
            }
            if (m_allow_parallel != m_settings->value(ALLOW_FILE_OP_PARALLEL).toBool())
            {
                m_allow_parallel = m_settings->value(ALLOW_FILE_OP_PARALLEL).toBool();
                FileOperationManager::getInstance()->setAllowParallel(m_allow_parallel);
            }
            if(m_showTrashDialog != m_settings->value(SHOW_TRASH_DIALOG).toBool())
            {
                m_showTrashDialog = m_settings->value(SHOW_TRASH_DIALOG).toBool();
                GlobalSettings::getInstance()->setValue("showTrashDialog", m_showTrashDialog);
                GlobalSettings::getInstance()->forceSync("showTrashDialog");
            }
        }
        m_user_dir_watcher->addPath(uri);
    });

}
// read user-dirs.dirs for all XDG standard path.
void UserdirManager::getUserdir()
{
    if(!m_current_user_dir.isEmpty())
    {
        m_last_user_dir.clear();
        m_last_user_dir = m_current_user_dir;
        m_current_user_dir.clear();
    }

    auto settings = new QSettings(m_user_path +"/.config/user-dirs.dirs",QSettings::IniFormat);
    settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

    m_current_user_dir.insert("XDG_DESKTOP_DIR",settings->value(QString("XDG_DESKTOP_DIR")).toString().replace("$HOME",m_user_path) + "/");
    m_current_user_dir.insert("XDG_DOWNLOAD_DIR",settings->value(QString("XDG_DOWNLOAD_DIR")).toString().replace("$HOME",m_user_path) + "/");
    m_current_user_dir.insert("XDG_TEMPLATES_DIR",settings->value(QString("XDG_TEMPLATES_DIR")).toString().replace("$HOME",m_user_path) + "/");
    m_current_user_dir.insert("XDG_PUBLICSHARE_DIR",settings->value(QString("XDG_PUBLICSHARE_DIR")).toString().replace("$HOME",m_user_path) + "/");
    m_current_user_dir.insert("XDG_DOCUMENTS_DIR",settings->value(QString("XDG_DOCUMENTS_DIR")).toString().replace("$HOME",m_user_path) + "/");
    m_current_user_dir.insert("XDG_MUSIC_DIR",settings->value(QString("XDG_MUSIC_DIR")).toString().replace("$HOME",m_user_path)+"/");
    m_current_user_dir.insert("XDG_PICTURES_DIR",settings->value(QString("XDG_PICTURES_DIR")).toString().replace("$HOME",m_user_path) + "/");
    m_current_user_dir.insert("XDG_VIDEOS_DIR",settings->value(QString("XDG_VIDEOS_DIR")).toString().replace("$HOME",m_user_path) + "/");

    //XDG_TEMPLATES_DIR will be used by right click menu.
    GlobalSettings::getInstance()->setValue(TEMPLATES_DIR,m_current_user_dir.value("XDG_TEMPLATES_DIR"));
    //fix non Chinese desktop enviroment templates empty issue, bug#42484
    //FIXME need system integration process
    //Comment to Fix bug#142431, English install bug
//    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
//    QString templateDir = homePath + "/模板/";
//    GlobalSettings::getInstance()->setValue(TEMPLATES_DIR, templateDir);

    settings->deleteLater();
}

//rename the old paths to overwrire new one.
void UserdirManager::moveFile()
{
    QMap<QString, QString>::const_iterator i;
    auto fileOpMgr = FileOperationManager::getInstance();
    for(i=m_current_user_dir.constBegin();i!=m_current_user_dir.constEnd();++i)
    {
        if(i.value()!=m_last_user_dir.value(i.key()))
        {
            //rename the old dir
            QDir *dir = new QDir(i.value());
            auto fileName = dir->dirName();
            QDir *lastDir = new QDir(m_last_user_dir.value(i.key()));

            if(lastDir->dirName() == QString(m_user_path)||lastDir->isEmpty()||!lastDir->exists())
                continue;

            auto operation = new Peony::FileRenameOperation("file://"+m_last_user_dir.value(i.key()),fileName);
            operation->setAutoOverwrite();
            fileOpMgr->startOperation(operation,false);
            if(i.key() == "XDG_DESKTOP_DIR")
            {
                QTimer::singleShot(500,[=](){
                    //refresh desktop
                    Q_EMIT desktopDirChanged();
                });
            }
        }
    }
}
