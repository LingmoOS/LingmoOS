// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import org.deepin.dtk 1.0 as D

Window {
    D.MessageManager.delegate: D.FloatingMessageContainer {
        id: msg
        panel: Rectangle {
            property string iconName: msg.message.iconName
            property string content: msg.message.content
        }
        duration: 1
    }
    property Component containerCom: Component {
        D.FloatingMessageContainer {
            id: msg2
            panel: Rectangle {
                property string iconName: msg2.message.icon
                property string content: msg2.message.msg
            }
            duration: 1
        }
    }
}
