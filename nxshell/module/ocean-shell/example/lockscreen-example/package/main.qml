// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4

import org.lingmo.ds 1.0

AppletItem {
    implicitWidth: 120
    implicitHeight: 100

    function lockscreenApplet()
    {
        var lockscreen = DS.applet("org.lingmo.ds.ocean-shutdown")
        if (lockscreen) {
            return lockscreen
        } else {
            console.warn("shutdown applet not found")
        }
    }

    Column {
        Button {
            text: "Lock"
            onClicked: {
                let lockscreen = lockscreenApplet()
                if (lockscreen) {
                    lockscreen.requestLock()
                }
            }
        }
        Button {
            text: "Shutdown"
            onClicked: {
                let lockscreen = lockscreenApplet()
                if (lockscreen) {
                    lockscreen.requestShutdown()
                }
            }
        }
        Button {
            text: "SwitchUser"
            onClicked: {
                let lockscreen = lockscreenApplet()
                if (lockscreen) {
                    lockscreen.requestSwitchUser()
                }
            }
        }
    }
}
