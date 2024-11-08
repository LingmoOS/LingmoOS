/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

import QtQuick 2.0
import org.lingmo.appwidget.private 1.0 as LINGMOAppWidget

Item {
    id:controlroot
    LINGMOAppWidget.Connection {
        id:appwidget
    }
    LINGMOAppWidget.Translator {
        id:translator
    }
    signal qmldefineeventchangedsignal(string eventname, string widgetname, var value);
    signal qmlclickedsignal(string widgetname, var value);
    signal update();
    signal gsettingUpdate(string key, var value);
    /* @deprecated 此信号接口会在应用同步后弃用 */
    signal userStatusChangedSignal(string status);
    property string appname
    property string username
    property var datavalue
    property string userstatus: "Normal"
    onAppnameChanged: {
        translator.installTranslators(appname)
    }

    onUserstatusChanged: {
        userStatusChangedSignal(userstatus)
    }

    Component.onCompleted: {
        appwidget.updatesignal.connect(controlroot.qmlUpdate)
        qmlclickedsignal.connect(appwidget.onClickedEvent)
        qmldefineeventchangedsignal.connect(appwidget.onDefineEvent)

        appwidget.gsettingChanged.connect(controlroot.gsettingChanged)
        appwidget.userStatusChanged.connect(controlroot.setUserStatus)
    }
    function qmlUpdate() {
        datavalue = appwidget.getData();
        update();
    }
    function register(appname, username)
    {
        appwidget.registerAppWidget(appname, username);
        console.log(" QML AppWidget register dbus");
    }
    function creatGsetting(gsettingname)
    {
        console.log(" QML AppWidget creatGsetting", gsettingname);
        if(appwidget.creatGsetting(gsettingname)) {
            console.log(" QML AppWidget register gsetting successful");
        }else {
            console.log(" QML AppWidget register gsetting fail");
        }
    }
    function deleteGsetting(gsettingname)
    {
        console.log(" QML AppWidget deleteGsetting", gsettingname);
        if(appwidget.deleteGsetting(gsettingname)) {
            console.log(" QML AppWidget delete gsetting successful");
        }else {
            console.log(" QML AppWidget delete gsetting fail");
        }
    }
    function getGsettingValue(gsettingname, key)
    {
        console.log(" QML AppWidget getGsettingValue", gsettingname, key, appwidget.getGsettingValue(gsettingname, key));
        return appwidget.getGsettingValue(gsettingname, key);
    }
    function gsettingChanged(key, value) {
        console.log(" QML AppWidget gsettingChanged", key,value)
        gsettingUpdate(key, value);
    }
    function setUserStatus(status) {
        console.log(" QML AppWidget setUserStatus", status, " appname: ", appwidget.appname)
        userstatus = status
    }
}
