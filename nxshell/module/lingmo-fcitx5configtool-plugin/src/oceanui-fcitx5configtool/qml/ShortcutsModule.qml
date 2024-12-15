// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0

OceanUIObject {
    readonly property var enumKeys: ["None", "CTRL_SHIFT", "ALT_SHIFT"]
    property var triggerKeys: oceanuiData.fcitx5ConfigProxy.globalConfigOption(
                                  "Hotkey", "TriggerKeys")
    property int enumerateForwardKeys: calculateEnumerateForwardKeys(
                                           oceanuiData.fcitx5ConfigProxy.globalConfigOption(
                                               "Hotkey",
                                               "EnumerateForwardKeys").value)

    function calculateEnumerateForwardKeys(value) {
        for (var i = 0; i < value.length; i++) {
            value[i] = String(value[i]).toUpperCase().replace("META", "SUPER")
            if (value[i].endsWith("_L") || value[i].endsWith("_R")) {
                value[i] = value[i].slice(0, -2)
            }
        }
        let formattedValue = value.length > 0 ? value.join("_") : ""
        return enumKeys.indexOf(formattedValue)
    }

    // title
    OceanUIObject {
        name: "ShortcutsTitle"
        parentName: "Manage Input Methods"
        displayName: qsTr("Shortcuts")
        weight: 210
        pageType: OceanUIObject.Item
        page: RowLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
            D.Button {
                id: resetBtn
                text: qsTr("Restore Defaults")
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.rightMargin: 10
                background: null
                textColor: D.Palette {
                    normal: D.DTK.makeColor(D.Color.Highlight)
                }
                onClicked: {
                    oceanuiData.fcitx5ConfigProxy.restoreDefault("Hotkey")
                    console.log("Restore Defaults button clicked")
                }
            }
        }
    }

    Component.onCompleted: {
        oceanuiData.fcitx5ConfigProxy.onRequestConfigFinished.connect(() => {
                                                                      triggerKeys = oceanuiData.fcitx5ConfigProxy.globalConfigOption(
                                                                          "Hotkey",
                                                                          "TriggerKeys")
                                                                      enumerateForwardKeys = calculateEnumerateForwardKeys(oceanuiData.fcitx5ConfigProxy.globalConfigOption("Hotkey", "EnumerateForwardKeys").value)
                                                                  })
    }

    function reverseEnumerateForwardKeys(index) {
        if (index < 0 || index >= enumKeys.length) {
            return ""
        }
        let value = enumKeys[index]
        if (value === "None") {
            return ""
        }
        let parts = value.split("_")
        for (var i = 0; i < parts.length; i++) {
            if (parts.length > 0) {
                parts[i] = parts[i].charAt(0).toUpperCase() + parts[i].slice(
                            1).toLowerCase()
            }
            if (parts[i] === "Super") {
                parts[i] = "Meta"
            } else if (parts[i] === "Shift") {
                parts[i] = "Shift_L"
            }
        }
        return parts
    }

    // Shortcut of scroll IM
    OceanUIObject {
        name: "scrollIM"
        parentName: "Manage Input Methods"
        weight: 220
        displayName: qsTr("Scroll between input methods")
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: D.ComboBox {
            id: comboBox
            flat: true
            model: enumKeys
            currentIndex: enumerateForwardKeys

            onCurrentIndexChanged: {
                console.log("Current index changed to:", currentIndex,
                            "with text:", model[currentIndex])
                oceanuiData.fcitx5ConfigProxy.setValue(
                            "Hotkey/EnumerateForwardKeys/0",
                            reverseEnumerateForwardKeys(currentIndex), true)
            }
        }
    }

    // Shortcut of turn on or off
    OceanUIObject {
        name: "turnGrouup"
        parentName: "Manage Input Methods"
        weight: 230
        pageType: OceanUIObject.Item
        page: OceanUIGroupView {}

        OceanUIObject {
            name: "shortcutSetting"
            parentName: "Manage Input Methods/turnGrouup"
            displayName: qsTr("Turn on or off input methods")
            weight: 231
            pageType: OceanUIObject.Editor
            page: D.KeySequenceEdit {
                placeholderText: qsTr("Please enter a new shortcut")
                keys: triggerKeys.value
                background: null

                onKeysChanged: {
                    if (keys.length > 0) {
                        oceanuiData.fcitx5ConfigProxy.setValue(
                                    "Hotkey/TriggerKeys/0", keys, true)
                    }
                }
            }
        }

        OceanUIObject {
            name: "shortcutSettingDesc"
            parentName: "Manage Input Methods/turnGrouup"
            weight: 232
            pageType: OceanUIObject.Item
            page: Label {
                leftPadding: 10
                rightPadding: 10
                Layout.fillWidth: true
                font: D.DTK.fontManager.t8
                opacity: 0.5
                wrapMode: Text.Wrap
                text: qsTr(
                          "It turns on or off the currently used input method."
                          + "If no input method is being used or the first input "
                          + "method is not the keyboard, it switches between the "
                          + "first input method and the currently used keyboard/input method.")
            }
        }
    }
}
