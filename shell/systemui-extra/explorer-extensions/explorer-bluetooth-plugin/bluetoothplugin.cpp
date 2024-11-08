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
 * Authors: tang guang <tangguang@kylinos.cn>
 *
 */

#include "bluetoothplugin.h"

#include <QFileInfo>
#include <QProcess>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusConnection>

#ifdef LINGMO_COMMON
#include <lingmosdk/lingmo-com4cxx.h>
#endif

#define V10_SP1_EDU "V10SP1-edu"

using namespace Peony;

static QString executeLinuxCmd(QString strCmd)
{
    QProcess p;
    p.start("/usr/bin/bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();
    return strResult.toLower();
}

static bool IsHuawei()
{
    QString str = executeLinuxCmd("cat /proc/cpuinfo | grep Hardware");
    if(str.length() == 0)
    {
        return false;
    }

    if(str.indexOf("huawei") != -1 || str.indexOf("pangu") != -1 ||
            str.indexOf("kirin") != -1)
    {
        return true;
    }
    else
    {
        return false;
    }
    return false;
}


static QString getDefaultAdapterAddr()
{
    QString reply_res;
    QDBusInterface iface("com.lingmo.bluetooth",
                         "/com/lingmo/bluetooth",
                         "com.lingmo.bluetooth",
                         QDBusConnection::sessionBus());

    QDBusPendingCall pcall = iface.asyncCall("getDefaultAdapterAddress");
    pcall.waitForFinished();

    QDBusMessage res = pcall.reply();
    if(res.type() == QDBusMessage::ReplyMessage)
    {
       if(res.arguments().size() > 0)
       {
           reply_res = res.arguments().takeFirst().toString();
           //qInfo() << reply_res;
       }
    }
    else
    {
       //qWarning() << res.errorName() << ": "<< res.errorMessage();
    }

    return reply_res;
}

BluetoothPlugin::BluetoothPlugin(QObject *parent)
{
    QTranslator *t = new QTranslator(this);
    qDebug()<<"system().name:"<<QLocale::system().name();
    qDebug()<<"\n\n\n\n\n\n\ntranslate:"<<t->load(":/translations/explorer-bluetooth-plugin_"+QLocale::system().name());
    QApplication::installTranslator(t);
}

QList<QAction *> BluetoothPlugin::menuActions(Peony::MenuPluginInterface::Types types, const QString &uri, const QStringList &selectionUris)
{
    qDebug() << Q_FUNC_INFO << uri << selectionUris.size();
    QList<QAction*> actions;

//    QProcess process;
//    process.start("/usr/sbin/rfkill list");
//    process.waitForFinished();
//    QByteArray output = process.readAllStandardOutput();
//    QString str_output = output;
//    if(!str_output.contains(QString("bluetooth"), Qt::CaseInsensitive))
//        return actions;
    QString m_defaultAdapterAddr = getDefaultAdapterAddr();
    if (m_defaultAdapterAddr.isEmpty() || 6 != m_defaultAdapterAddr.split(":").size())
    {
        qInfo() << Q_FUNC_INFO << m_defaultAdapterAddr << endl;
        return actions;
    }

    //华为机型不提供文件发送功能
    if(IsHuawei()){
        return actions;
    }

    if(!QFileInfo::exists("/usr/bin/lingmo-bluetooth")){
        return actions;
    }
    if (types == MenuPluginInterface::DirectoryView || types == MenuPluginInterface::DesktopWindow)
    {
        if (! selectionUris.isEmpty()) {
            auto info = FileInfo::fromUri(selectionUris.first());
            //special type mountable, return
            qDebug()<<"info isVirtual:"<<info->isVirtual()<<info->mimeType();
            if (!selectionUris.first().startsWith("file:///"))
                  return actions;
            else{
                if(info->mimeType().split("/").at(1) != "directory"){
                    QAction *compress = new QAction(QIcon::fromTheme("blueman-tray"), tr("Send from bluetooth to..."), nullptr);
                    actions << compress;
                    connect(compress, &QAction::triggered, [=](){
                        QStringList target;
                        for (auto& str : selectionUris) {
                            QString strDecode = Peony::FileUtils::urlDecode(str);
                            qDebug() << Q_FUNC_INFO << str << "   =   " << strDecode;
                            target << strDecode;
                        }
                        QDBusMessage m = QDBusMessage::createMethodCall("com.lingmo.bluetooth","/com/lingmo/bluetooth","com.lingmo.bluetooth","setSendTransferFileMesg");
                        m << target;
                        qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
                        // 发送Message
                        QDBusConnection::sessionBus().call(m);
                    });
                }
            }
        }
    }

    return actions;
}
