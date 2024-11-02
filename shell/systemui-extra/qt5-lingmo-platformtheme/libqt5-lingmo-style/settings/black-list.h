/*
 * Qt5-LINGMO's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef BLACKLIST_H
#define BLACKLIST_H

#include <QStringList>

static const QStringList blackAppList() {
    QStringList l;
    //l<<"lingmo-control-center";
    l<<"lingmo-software-center.py";
    l<<"ubuntu-lingmo-software-center.py";
//    l<<"lingmo-burner";
    l<<"assistant";
    l<<"sogouIme-configtool";
    l<<"Ime Setting";
//    l<<"lingmo-user-guide";
    l<<"biometric-authentication";
    l<<"qtcreator";

    return l;
}

static const QStringList blackAppListWithBlurHelper() {
    QStringList l;
    l<<"youker-assistant";
//    l<<"lingmo-assistant";
//    l<<"lingmo-video";
//    l<<"lingmo-control-center";
    l<<"ubuntu-lingmo-software-center.py";
//    l<<"lingmo-burner";
    l<<"lingmo-clipboard";
    return l;
}

static const QStringList useDarkPaletteList() {
    //use dark palette in default style.
    QStringList l;
//    l<<"lingmo-menu";
//    l<<"lingmo-panel";
//    l<<"lingmo-sidebar";
//    l<<"lingmo-volume-control-applet-qt";
//    l<<"lingmo-nm";                              //网络
//    l<<"panellingmo-panel";                       //日历
//    l<<"lingmo-power-manager-tray";
////    l<<"lingmo-bluetooth";
//    l<<"sogouimebs";                            //输入法
//    l<<"lingmo-device-daemon";                   //U盘
    l<<"lingmo-flash-disk";
//    l<<"lingmo-bluetooth";
    l<<"mktip";
    l<<"lingmo-video";

    return l;
}

static const QStringList useDefaultPaletteList() {
    //use light palette
    QStringList l;
//    l<<"kybackup";
//    l<<"biometric-manager";
//    l<<"lingmo-video";

    return l;
}

static const QStringList useTransparentButtonList() {
    //use transparent button
    QStringList l;
//    l<<"kybackup";
//    l<<"biometric-manager";
    l<<"lingmo-video";
    l<<"lingmo-ipmsg";
    l<<"lingmo-weather";
    l<<"lingmo-notebook";

//    l<<"lingmo-recorder";

    return l;
}

static const QStringList focusStateActiveList() {
    //focus state style
    QStringList l;
    l<<"lingmo-menu";
    l<<"platformthemeDemo";
    l<<"lingmo-greeter-dialog";
    l<<"lingmo-screensaver";

    return l;
}

static const QStringList windowManageBlackList() {
    QStringList l;
    l << "iflyime-spe-sym";
    l << "iflyime-qimpanel";
    l << "iflyime-setw";
    l << "iflyime-sett";
    l << "iflyime-qim";
    l << "iflyime-hw";
    l << "SpecificSymbol";
    return l;
}

#endif // BLACKLIST_H
