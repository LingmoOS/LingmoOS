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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 * Authors: Meihong <hemeihong@kylinos.cn>
 *
 */

#include "admin-menu-plugin.h"
#include "file-info.h"

#include <QProcess>
#include <QtConcurrent>
#include <QUrl>
#include <QTranslator>
#include <QApplication>
#include <QFile>

#include <QtGlobal>

#include <QDebug>

using namespace Peony;

AdminMenuPlugin::AdminMenuPlugin(QObject *parent) : QObject(parent)
{
    QTranslator *t = new QTranslator(this);

    bool b_load = t->load(":/translations/explorer-admin_"+QLocale::system().name());
    qDebug()<<"\n\n\n\n\n\n\n  AdminMenuPlugin translate:"<<b_load;
    QFile file(":/translations/explorer-admin_"+QLocale::system().name()+".ts");
    qDebug()<<"file:"<<file.exists();
    QApplication::installTranslator(t);
}

QList<QAction *> AdminMenuPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    QList<QAction *> l;

    if (uri.startsWith("mtp://")) {
        return l;
    }

    if (selectionUris.isEmpty()) {
        auto directoryAction = new QAction(tr("Open Directory as Admin"));
        l<<directoryAction;
        directoryAction->connect(directoryAction, &QAction::triggered, [=](){
            QtConcurrent::run([=](){
                QProcess p;
                QUrl url = uri;
                p.setProgram("pkexec");
                QStringList args;
                args<<"explorer"<<url.toEncoded();
                auto env = qgetenv("QT_QPA_PLATFORMTHEME");
                if (!env.isEmpty())
                    args<<"-platformtheme"<<env;
                p.setArguments(args);
                p.start();
                p.waitForFinished();
            });
        });
    } else if (selectionUris.count() == 1) {
        auto info = FileInfo::fromUri(selectionUris.first());
        //special type mountable, return
        qDebug()<<"info isVirtual:"<<info->isVirtual()<<info->mimeType();
        if (selectionUris.first().startsWith("computer:///") || 
            selectionUris.first().startsWith("trash:///")|| 
            info->isVirtual())
            return l;
        if (info->isDir()) {
            auto directoryAction = new QAction(tr("Open Directory as Admin"));
            l<<directoryAction;
            directoryAction->connect(directoryAction, &QAction::triggered, [=](){
                QtConcurrent::run([=](){
                    QStringList args;
                    QUrl url = selectionUris.first();
                    args<<"explorer"<<url.toEncoded();
                    auto env = qgetenv("QT_QPA_PLATFORMTHEME");
                    if (!env.isEmpty())
                        args<<"-platformtheme"<<env;
                    QProcess p;
                    p.setProgram("pkexec");
                    p.setArguments(args);
                    p.start();
                    p.waitForFinished();
                });
            });
        }
        if (info->mimeType().startsWith("text")) {
            auto directoryAction = new QAction(tr("Open Text as Admin"));
            l<<directoryAction;
            directoryAction->connect(directoryAction, &QAction::triggered, [=](){
                QtConcurrent::run([=](){
                    QProcess p;
                    QUrl url = selectionUris.first();
                    p.setProgram("pkexec");
                    p.setArguments(QStringList()<<"pluma"<<url.toEncoded());
                    p.start();
                    p.waitForFinished();
                });
            });
        }
    }
#else
#endif
    return l;
}
