// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3

import org.lingmo.oceanui 1.0

OceanUIObject {
    OceanUIObject {
        name: "KeyboardCommon"
        parentName: "keyboard"
        displayName: qsTr("Common")
        weight: 10
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {
            spacing: 5
            isGroup: false
            height: implicitHeight + 20
        }
        Common {}
    }
    OceanUIObject {
        name: "KeyboardLayout"
        parentName: "keyboard"
        displayName: qsTr("Keyboard layout")
        description: qsTr("Set system default keyboard layout")
        icon: "oceanui_nav_keyboard"
        weight: 100
        page: OceanUIRightView {
            spacing: 5
        }
        KeyboardLayout {}
    }

    // 101~299 for InputMethod

    Shortcuts {
        weight: 300
    }
}
