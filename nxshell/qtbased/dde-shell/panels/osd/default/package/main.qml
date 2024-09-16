// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.deepin.ds 1.0
import org.deepin.dtk 1.0 as D

AppletItem {
    id: control
    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height

    function update(osdType)
    {
        if (match(osdType)) {
            selectModleByType(osdType)
            return true
        }
        return false
    }
    function match(osdType)
    {
        for (var i = 0; i < osdTypeModel.count; i++) {
            var item = osdTypeModel.get(i)
            if (item.type === osdType) {
                return true
            }
        }
        return false
    }

    function selectModleByType(osdType)
    {
        for (var i = 0; i < osdTypeModel.count; i++) {
            var item = osdTypeModel.get(i)
            if (item.type === osdType) {
                iconName = item.iconName
                text = item.text
                break
            }
        }
    }

    property string iconName
    property string text
    ListModel {
        id: osdTypeModel

        ListElement { type: "WLANOn"; iconName: "osd_wifi_on"; text: qsTr("WLAN on")}
        ListElement { type: "WLANOff"; iconName: "osd_wifi_off"; text: qsTr("WLAN off")}
        ListElement { type: "CapsLockOn"; iconName: "osd_capslock_on"; text: qsTr("Caps Lock on")} // 大写
        ListElement { type: "CapsLockOff"; iconName: "osd_capslock_off"; text: qsTr("Caps Lock off")} // 小写
        ListElement { type: "NumLockOn"; iconName: "osd_keyboard_on"; text: qsTr("Numeric keypad on")} // 数字键盘开启
        ListElement { type: "NumLockOff"; iconName: "osd_keyboard_off"; text: qsTr("Numeric keypad off")} // 数字键盘关闭
        ListElement { type: "TouchpadOn"; iconName: "osd_touchpad_on"; text: qsTr("Touchpad on")} // 触摸板开启
        ListElement { type: "TouchpadOff"; iconName: "osd_touchpad_off"; text: qsTr("Touchpad off")} // 触摸板关闭
        ListElement { type: "TouchpadToggle"; iconName: "osd_touchpad_exchange"; text: qsTr("Touchpad toggle")} // 触控板切换
        ListElement { type: "FnToggle"; iconName: "osd_fn"; text: qsTr("Fn toggle")} // Fn切换
        ListElement { type: "AirplaneModeOn"; iconName: "osd_airplane_mode_on"; text: qsTr("Airplane mode on")}
        ListElement { type: "AirplaneModeOff"; iconName: "osd_airplane_mode_off"; text: qsTr("Airplane mode off")}
        ListElement { type: "AudioMicMuteOn"; iconName: "osd_mic_on"; text: qsTr("Mute on")}
        ListElement { type: "AudioMicMuteOff"; iconName: "osd_mic_off"; text: qsTr("Mute off")}
        ListElement { type: "balance"; iconName: "osd_power_balance"; text: qsTr("Balanced power")}
        ListElement { type: "powersave"; iconName: "osd_power_save"; text: qsTr("Power saver")}
        ListElement { type: "performance"; iconName: "osd_power_performance"; text: qsTr("High performance")}
        ListElement { type: "SwitchWM3D"; iconName: "osd_wm_3d"; text: qsTr("Window effect enabled")}
        ListElement { type: "SwitchWM2D"; iconName: "osd_wm_2d"; text: qsTr("Window effect disabled")}
        ListElement { type: "SwitchWMError"; iconName: "osd_wm_failed"; text: qsTr("Failed to enable window effects")}
    }

    RowLayout {
        anchors.leftMargin: 68
        anchors.rightMargin: 26

        D.DciIcon {
            sourceSize {
                width: 32
                height: 32
            }
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 14
            visible: control.iconName
            name: control.iconName
        }

        Text {
            Layout.fillWidth: true
            Layout.leftMargin: 68
            font: D.DTK.fontManager.t4
            Layout.alignment: Qt.AlignVCenter
            visible: control.text
            text: control.text
        }
    }
}
