/*
 * liblingmosdk-widgetutils's Library
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kwidgetutils.h"
#include <QApplication>
#include <QGuiApplication>
#include <QDebug>
#include <QtDBus/QtDBus>
#include <QGSettings/QGSettings>
#include <QByteArray>
#include <QVariant>

#define KWIN_SERVICE "org.lingmo.KWin"
#define KWIN_PATH "/Compositor"
#define KWIN_INTERFACE "org.lingmo.kwin.Compositing"
using namespace kdk;

KWidgetUtils::KWidgetUtils()
{

}

void KWidgetUtils::highDpiScaling()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    qDebug()<<"setAttribute success";
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    qDebug()<<"setHighDpiScaleFactorRoundingPolicy success";
#endif
}

bool KWidgetUtils::checkCompositorRunning()
{
    QDBusInterface dbus_iface (KWIN_SERVICE, KWIN_PATH, KWIN_INTERFACE, QDBusConnection::sessionBus());
    QVariant reply = dbus_iface.property ("active");
    bool isfoundCompositingManger = false;
    if (reply.toBool()) {
        isfoundCompositingManger = true;
    }

    if(!isfoundCompositingManger)
    {
        if(QGSettings::isSchemaInstalled("org.gnome.metacity"))
        {
            QProcess process;
            process.start("sh -c \"ps -e |grep metacity\"");
            if(process.waitForStarted(100)&&process.waitForFinished(100) && process.readAllStandardOutput().contains("metacity"))
            {
                QGSettings metacityGSettings("org.gnome.metacity", "/org/gnome/metacity/");
                isfoundCompositingManger = metacityGSettings.get("compositing-manager").toBool();
            }
        }
    }

    if(!isfoundCompositingManger)
    {
        if(QGSettings::isSchemaInstalled("org.mate.Marco.general"))
        {
            QProcess process;
            process.start("sh -c \"ps -e |grep marco\"");
            if(process.waitForStarted(100)&&process.waitForFinished(100) && process.readAllStandardOutput().contains("marco"))
            {
                QGSettings marcoGSettings("org.mate.Marco.general", "/org/mate/marco/general/");
                isfoundCompositingManger = marcoGSettings.get("compositing-manager").toBool();
            }
        }
    }

    return isfoundCompositingManger;
}
