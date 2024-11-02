/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "bluetooth.h"

Environment envPC = Environment::NOMAL;
bool global_rightToleft = false;

Bluetooth::Bluetooth() : mFirstLoad(true) {

    KyDebug() << "start" << "++ukccBluetooth========================";
    QStringList addrList = BlueToothDBusService::getAllAdapterAddress();
    if (!addrList.size())
        ukccbluetoothconfig::ukccGsetting->set("show",QVariant(false));
    else
        ukccbluetoothconfig::ukccGsetting->set("show",QVariant(true));

    KyDebug() << envPC ;

    QTranslator * translator = new QTranslator(this);
    translator->load("/usr/share/lingmo-bluetooth/translations/ukcc-bluetooth_" + QLocale::system().name() + ".qm");
    QApplication::installTranslator(translator);

    pluginName = tr("Bluetooth");
    pluginType = DEVICES;

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        KyInfo() << "global_rightToleft set true";
        global_rightToleft = true;
    }
}

Bluetooth::~Bluetooth() {
    if (!mFirstLoad) {
        pluginWidget->deleteLater();
    }
}

QString Bluetooth::plugini18nName() {
    return pluginName;
}

int Bluetooth::pluginTypes() {
    return pluginType;
}

QWidget *Bluetooth::pluginUi() {

    if (mFirstLoad) {
        mFirstLoad = false;
        pluginWidget = new BlueToothMainWindow;
    }

    if (!mFirstLoad && (nullptr != pluginWidget))
    {
        BlueToothDBusService::registerClient();
    }

    return pluginWidget;
}

const QString Bluetooth::name() const {
    return QStringLiteral("Bluetooth");
}

bool Bluetooth::isShowOnHomePage() const
{
    return true;
}

QIcon Bluetooth::icon() const
{
    return QIcon::fromTheme("bluetooth-active-symbolic");
}

bool Bluetooth::isEnable() const
{
    QStringList addrList = BlueToothDBusService::getAllAdapterAddress();
    KyDebug() << addrList ;
    if (addrList.size())
    {
        KyInfo() << "Bluetooth::isEnable is true";
        return true;
    }
    else
    {
        KyInfo() << "Bluetooth::isEnable is false";
        return false;
    }
}

void Bluetooth::plugin_leave()
{
    if (nullptr != pluginWidget)
    {
        //界面切出时注销控制面板注册
        BlueToothDBusService::unregisterClient();
    }
}
