import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import LingmoUI

LingmoScrollablePage {
    title: qsTr("Hotkey Picker")

    LingmoFrame {
        Layout.fillWidth: true
        Layout.preferredHeight: childrenRect.height
        leftPadding: 10
        ColumnLayout{
            anchors.verticalCenter: parent.verticalCenter
            Item{
                Layout.preferredHeight: 15
            }
            Repeater{
                model: LingmoApp.launcher.hotkeys.children
                delegate: LingmoHotkeyPicker{
                    text: model.name
                    syncHotkey: LingmoApp.launcher.hotkeys.children[index]
                    Layout.leftMargin: 15
                }
            }
            Item{
                Layout.preferredHeight: 15
            }
        }
    }

}