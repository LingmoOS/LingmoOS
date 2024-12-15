// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: keyboard
    OceanUIObject {
        name: "keyboard"
        parentName: "device"
        displayName: qsTr("Keyboard")
        description: qsTr("General Settings, keyboard layout, input method, shortcuts")
        icon: "oceanui_nav_keyboard"
        weight: 40
    }

    visible: false
    OceanUIDBusInterface {
        property var numLockState
        service: "org.lingmo.ocean.Keybinding1"
        path: "/org/lingmo/ocean/Keybinding1"
        inter: "org.lingmo.ocean.Keybinding1"
        connection: OceanUIDBusInterface.SessionBus
        onNumLockStateChanged: keyboard.visible = true
    }
}
