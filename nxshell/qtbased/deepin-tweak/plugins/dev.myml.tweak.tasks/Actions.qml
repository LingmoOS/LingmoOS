// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import Nemo.DBus 2.0

Item {
    DBusInterface {
        id: sessionManager
        service: 'com.deepin.SessionManager'
        iface: 'com.deepin.SessionManager'
        path: '/com/deepin/SessionManager'
    }
    ListModel {
        id: model
        ListElement {
            title: qsTr("Shutdown");
            run: () => {
                console.log("shutdown")
                sessionManager.call("RequestShutdown")
            }
        }
        ListElement {
            title: qsTr("Reboot");
            run: () => {
                console.log("reboot")
                sessionManager.call("RequestReboot")
            }
        }
        ListElement {
            title: qsTr("Lock");
            run: () => {
                console.log("lock")
                sessionManager.call("RequestLock")
            }
        }
        ListElement {
            title: qsTr("Logout");
            run: () => {
                console.log("logout")
                sessionManager.call("RequestLogout")
            }
        }
    }
    function getModel(){
        return model
    }
    function execAction(index) {
        model.get(index).run()
    }
}