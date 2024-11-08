/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include <QCoreApplication>
#include <QtWebChannel>
#include <QtWebSockets>
#include <QSettings>
#include "shared/websocketclientwrapper.h"
#include "shared/websockettransport.h"
#include "lingmosdk/bios.h"
//#include "lingmosdk/dependence.h"
#include "lingmosdk/disk.h"
#include "lingmosdk/gps.h"
#include "lingmosdk/mainboard.h"
#include "lingmosdk/net.h"
#include "lingmosdk/netcard.h"
#include "lingmosdk/peripheralsenum.h"
#include "lingmosdk/process.h"
#include "lingmosdk/resolutionctl.h"
#include "lingmosdk/runinfo.h"
#include "lingmosdk/cpuinfo.h"
#include "lingmosdk/packageinfo.h"
#include "lingmosdk/resource.h"
#include "lingmosdk/sysinfo.h"
#include "lingmosdk/print.h"

#define CONFIG_FILE_NAME "/etc/websocket/lingmosdk-dbus-websocket.conf"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSettings settings(CONFIG_FILE_NAME, QSettings::IniFormat);
    settings.beginGroup("WebSocketServer");
    int port = settings.value("port", 12345).toInt();
    settings.endGroup();

    QWebSocketServer server(QStringLiteral("WebSocket Setup !"), QWebSocketServer::NonSecureMode);
    if (!server.listen(QHostAddress::LocalHost, port))
    {
        qFatal("Failed to open web socket server.");
    }
    qDebug() << "port=" << port;

    //wrap WebSocket clients in QWebChannelAbstractTransport objects
    WebSocketClientWrapper clientWrapper(&server);

    // setup the channel
    QWebChannel channel(nullptr);
    QObject::connect(&clientWrapper, &WebSocketClientWrapper::clientConnected,
                     &channel, &QWebChannel::connectTo);

    Bios bios;
    //Dependence dependence;
    Cpuinfo cpuinfo;
    Disk disk;
    Gps gps;
    Mainboard mainboard;
    Net net;
    NetCard netcard;
    Packageinfo packageinfo;
    Peripheralsenum peripheralsenum;
    Print print;
    Process process;
    Resolutionctl resolutionctl;
    Resource resource;
    Runinfo runinfo;
    Sysinfo sysinfo;

    QHash<QString, QObject *> hash;
    hash["bios"] = &bios;
    hash["cpuinfo"] = &cpuinfo;
    //hash["dependence"] = &dependence;
    hash["disk"] = &disk;
    hash["gps"] = &gps;
    hash["mainboard"] = &mainboard;
    hash["net"] = &net;
    hash["netcard"] = &netcard;
    hash["packageinfo"] = &packageinfo;
    hash["peripheralsenum"] = &peripheralsenum;
    hash["print"] = &print;
    hash["process"] = &process;
    hash["resolutionctl"] = &resolutionctl;
    hash["resource"] = &resource;
    hash["runinfo"] = &runinfo;
    hash["sysinfo"] = &sysinfo;
    channel.registerObjects(hash);

    //Widget w;
//    w.show();

//    Screenctl s;
//    s.Open();
//    s.BeginSign();
    return a.exec();
}
