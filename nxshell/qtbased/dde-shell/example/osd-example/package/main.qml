// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.15

import org.deepin.ds 1.0

AppletItem {
    objectName: "osd example"
    implicitWidth: 400
    implicitHeight: 200

    GridView {
        anchors.fill: parent
        cellHeight: 40
        property var audioTypes: ["AudioUp", "AudioDown", "AudioMute", "AudioUpAsh", "AudioDownAsh", "AudioMuteAsh"]
        property var brightnessTypes: ["BrightnessUp", "BrightnessDown", "BrightnessUpAsh", "BrightnessDownAsh"]
        property var defaultTypes: ["WLANOn", "WLANOff", "CapsLockOn", "CapsLockOff", "NumLockOn", "NumLockOff", "TouchpadOn", "TouchpadOff", "TouchpadToggle", "FnToggle", "AirplaneModeOn", "AirplaneModeOff", "AudioMicMuteOn", "AudioMicMuteOff", "balance", "powersave", "performance", "SwitchWM3D", "SwitchWM2D", "SwitchWMError"]
        property var displayTypes: ["SwitchMonitors"]
        property var kblayoutTypes: ["SwitchLayout"]

        property var types: []

        model: types
        delegate: Button {
            text: model.modelData
            onClicked: Applet.sendOsd(modelData)
        }
        Component.onCompleted: {
            var tmp = types.concat(audioTypes)
            tmp = tmp.concat(brightnessTypes)
            tmp = tmp.concat(defaultTypes)
            tmp = tmp.concat(displayTypes)
            tmp = tmp.concat(kblayoutTypes)
            types = tmp
        }
    }
}
