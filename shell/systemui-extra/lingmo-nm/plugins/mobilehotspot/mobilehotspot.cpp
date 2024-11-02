/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "mobilehotspot.h"


#include <QDebug>
#include <QVariant>

MobileHotspot::MobileHotspot() :  mFirstLoad(true) {

    QTranslator* translator = new QTranslator(this);
    qDebug() << "/usr/share/lingmo-nm/mobilehotspot/" + QLocale::system().name();
    if (!translator->load("/usr/share/lingmo-nm/mobilehotspot/" + QLocale::system().name())) {
        qDebug() << "load failed";
    }
    QApplication::installTranslator(translator);

    pluginName = tr("MobileHotspot");
    qDebug() << pluginName;
    pluginType = NETWORK;

    qDBusRegisterMetaType<QMap<QString, bool> >();
    qDBusRegisterMetaType<QMap<QString, int> >();
    needLoad = isExitWirelessDevice();
}

MobileHotspot::~MobileHotspot()
{
    if (!mFirstLoad) {
//        delete pluginWidget;
//        pluginWidget = nullptr;

    }
}

QString MobileHotspot::plugini18nName() {
    return pluginName;
}

int MobileHotspot::pluginTypes() {
    return pluginType;
}

QWidget *MobileHotspot::pluginUi() {
    if (mFirstLoad) {
        mFirstLoad = false;

        pluginWidget = new MobileHotspotWidget;
        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    }
    return pluginWidget;
}

const QString MobileHotspot::name() const {

    return QStringLiteral("mobilehotspot");
}

bool MobileHotspot::isEnable() const
{
    qDebug() << needLoad;
    return needLoad;
}


bool MobileHotspot::isShowOnHomePage() const
{
    return false;
}

QIcon MobileHotspot::icon() const
{
    return QIcon::fromTheme("lingmo-hotspot-symbolic");
}

QString MobileHotspot::translationPath() const
{
    return "/usr/share/lingmo-nm/mobilehotspot/%1.ts";
}

void MobileHotspot::initSearchText()
{
    //~ contents_path /mobilehotspot/Mobile Hotspot
    tr("Mobile Hotspot");
    //~ contents_path /mobilehotspot/Open mobile hotspot
    tr("Open mobile hotspot");
    //~ contents_path /mobilehotspot/Wi-Fi Name
    tr("Wi-Fi Name");
    //~ contents_path /mobilehotspot/Network Password
    tr("Network Password");
    //~ contents_path /mobilehotspot/Network Frequency band
    tr("Network Frequency band");
    //~ contents_path /mobilehotspot/Shared NIC port
    tr("Shared NIC port");
}

bool MobileHotspot::isExitWirelessDevice()
{
    QDBusInterface *interface = new QDBusInterface("com.lingmo.network", "/com/lingmo/network",
                                     "com.lingmo.network",
                                     QDBusConnection::sessionBus());
    if (!interface->isValid()) {
        qDebug() << "/com/lingmo/network is invalid";
        return false;
    }

    QDBusReply<QVariantMap> reply = interface->call(QStringLiteral("getDeviceListAndEnabled"),1);
    if(!reply.isValid()) {
        qWarning() << "getWirelessDeviceList error:" << reply.error().message();
        return false;
    }

    QMap<QString, bool> deviceListMap;
    QVariantMap::const_iterator itemIter = reply.value().cbegin();
    while (itemIter != reply.value().cend()) {
        deviceListMap.insert(itemIter.key(), itemIter.value().toBool());
        itemIter ++;
    }

    QDBusReply<QVariantMap> capReply = interface->call("getWirelessDeviceCap");
    if (!capReply.isValid()) {
        qDebug()<<"execute dbus method 'getWirelessDeviceCap' is invalid in func initInterfaceInfo()" <<capReply.error().type() ;
        return false;
    }

    QMap<QString, int> devCapMap;
    QVariantMap::const_iterator item = reply.value().cbegin();
    while (item != reply.value().cend()) {
        devCapMap.insert(item.key(), item.value().toInt());
        item ++;
    }

    if (deviceListMap.isEmpty()) {
        qDebug() << "no wireless device";
        return false;
    } else {
        QMap<QString, bool>::Iterator iter = deviceListMap.begin();
        while (iter != deviceListMap.end()) {
            QString interfaceName = iter.key();
            if (!devCapMap.contains(interfaceName)) {
                iter++;
                continue;
            }
            if (deviceListMap[interfaceName] & 0x01) {
                qDebug() << "wireless device" <<  interfaceName << "support hotspot";
                return true;
            }
            iter++;
        }
    }
    qDebug() << "no wireless device support hotspot";
    return false;
}

