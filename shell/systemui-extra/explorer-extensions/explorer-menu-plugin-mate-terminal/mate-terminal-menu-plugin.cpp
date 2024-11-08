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
 *
 */

#include "mate-terminal-menu-plugin.h"
#include <file-info.h>
#include <gio/gio.h>

#include <QAction>
#include <QUrl>

#include <QTranslator>
#include <QLocale>
#include <QApplication>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

#include <QtConcurrent>
#include <QProcess>
#include <QMessageBox>

#include <QDebug>

#ifdef LINGMO_COMMON
#include <lingmosdk/lingmo-com4cxx.h>
#endif

#define V10_SP1_EDU "V10SP1-edu"

using namespace Peony;

static QString terminal_cmd = nullptr;
static QString m_uri = nullptr;

MateTerminalMenuPlugin::MateTerminalMenuPlugin(QObject *parent) : QObject (parent)
{
    QTranslator *t = new QTranslator(this);
    qDebug()<<"\n\n\n\n\n\n\ntranslate:"<<t->load(":/translations/explorer-mate-terminal-extension_"+QLocale::system().name());
    QApplication::installTranslator(t);

    QtConcurrent::run([=]{
        GList *infos = g_app_info_get_all();
        GList *l = infos;
        while (l) {
            const char *cmd = g_app_info_get_executable(static_cast<GAppInfo*>(l->data));
            QString tmp = cmd;
            if (tmp.contains("terminal") || tmp.contains("terminator")) {
                qDebug() << "terminal cmd:" <<tmp;
                //if has mate-terminal, use it first
                if (tmp == "mate-terminal") {
                    terminal_cmd = tmp;
                    break;
                }
                else if (terminal_cmd == "terminator"){
                    //fix endless loop issue
                    l = l->next;
                    continue;
                }
                else
                    terminal_cmd = tmp;
            }
            l = l->next;
        }
        g_list_free_full(infos, g_object_unref);
    });
}

void MateTerminalMenuPlugin::openTerminal(){
    //qDebug()<<"triggered"<<uri;
    QUrl url = m_uri;
    auto directory = url.path();

    if (g_file_test ("/usr/bin/lingmo-process-manager", G_FILE_TEST_EXISTS)) {
        QDBusMessage msg = QDBusMessage::createMethodCall("com.lingmo.ProcessManager", "/com/lingmo/ProcessManager/AppLauncher", "com.lingmo.ProcessManager.AppLauncher", "RunCommand");
        gchar* quote_path = g_shell_quote(directory.toUtf8().constData());
        QString cmd = QString("/usr/bin/mate-terminal --working-directory=\"%1\"").arg(directory);
        msg << QString(cmd);
        g_free (quote_path);
        QDBusConnection::sessionBus().asyncCall(msg);
        return;
    }

    gchar **argv = nullptr;
    g_shell_parse_argv (terminal_cmd.toUtf8().constData(), nullptr, &argv, nullptr);
    GError *err = nullptr;
    g_spawn_async (directory.toUtf8().constData(),
                   argv,
                   nullptr,
                   G_SPAWN_SEARCH_PATH,
                   nullptr,
                   nullptr,
                   nullptr,
                   &err);
    if (err) {
        qDebug()<<"openTerminal failed:"<<err->message;
        g_error_free(err);
        err = nullptr;
        //try again to open terminal
        //maybe this method is better,but still need more test to confirm
        tryOpenAgain();
    }
    g_strfreev (argv);
}

void MateTerminalMenuPlugin::tryOpenAgain()
{
    GList *infos = g_app_info_get_all();
    GList *l = infos;
    while (l) {
        const char *cmd = g_app_info_get_executable(static_cast<GAppInfo*>(l->data));
        QString tmp = cmd;
        if (tmp.contains("terminal") || tmp.contains("terminator")) {
            qDebug() << "terminal cmd:" <<tmp;
            //if has mate-terminal, use it first
            if (tmp == "mate-terminal") {
                terminal_cmd = tmp;
                break;
            }
            else if (terminal_cmd == "terminator"){
                //fix endless loop issue
                l = l->next;
                continue;
            }
            else
                terminal_cmd = tmp;
        }
        l = l->next;
    }
    g_list_free_full(infos, g_object_unref);
    QUrl url = m_uri;
    auto absPath = url.path();
    qDebug() << "tryOpenAgain terminal url:" <<absPath;
    QProcess p;
    QString trueCmd = terminal_cmd;
    if (! trueCmd.startsWith("/usr"))
        trueCmd = "/usr/bin/" + trueCmd;
    p.setProgram(trueCmd);
    p.setArguments(QStringList()<<"--working-directory"<<absPath);
//    p.startDetached();
    bool result = p.startDetached(p.program(), p.arguments());
    p.waitForFinished(-1);

    if (! result)
    {
        QMessageBox::critical(nullptr, QObject::tr("Open terminal fail"),
                              QObject::tr("Open terminal failed, did you removed the default terminal? "
                                          " If it's true please reinstall it."));
    }
}

QList<QAction *> MateTerminalMenuPlugin::menuActions(Types types, const QString &uri, const QStringList &selectionUris)
{
#ifdef LINGMO_COMMON
    if (QString::fromStdString(KDKGetPrjCodeName()) == V10_SP1_EDU) {
        return QList<QAction*>();
    }
#endif

    QList<QAction *> actions;
    qDebug() << "terminal_cmd:" <<terminal_cmd <<uri;
    if (uri.startsWith("trash://") || uri.startsWith("recent://"))
    {
        return actions;
    }

    //set default terminal_cmd value for unfind terminal has no option issue
    //FIXME for unknow reason, did not find any terminal but it is actually exist
    if (terminal_cmd.isNull())
    {
        qWarning("open terminal option has not find any terminal, set as default value.");
        terminal_cmd = "mate-terminal";
    }

    if (types == MenuPluginInterface::DirectoryView || types == MenuPluginInterface::DesktopWindow) {
        if (selectionUris.isEmpty()) {
            m_uri = uri;
            //virtual path not show this option
            auto info = FileInfo::fromUri(uri);
            if (info->isVirtual())
                return actions;

            QAction *dirAction = new QAction(QIcon::fromTheme("utilities-terminal"), tr("Open Directory in Terminal"), nullptr);
            dirAction->connect(dirAction, &QAction::triggered, [=](){
                openTerminal();
            });
            actions<<dirAction;
        }
        if (selectionUris.count() == 1) {
            //select computer or trash, return
            if (selectionUris.first().startsWith("trash://")
                || selectionUris.first().startsWith("computer://"))
                return actions;

            //virtual path not show this option
            auto info = FileInfo::fromUri(selectionUris.first());
            if (info->isVirtual())
                return actions;

            if (info->isDir()) {
                m_uri = selectionUris.first();
                QAction *dirAction = new QAction(QIcon::fromTheme("utilities-terminal"), tr("Open Directory in Terminal"), nullptr);
                dirAction->connect(dirAction, &QAction::triggered, [=](){
                    openTerminal();
                });
                actions<<dirAction;
            }
        }
    }
    return actions;
}
