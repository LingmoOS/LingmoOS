// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0

DialogWindow {
    id: deviceLoadDlg
    modality: Qt.WindowModal
    width: 422
    property int lblHeight: 77
    property int btnWidth: 190
    icon: "deepin-album"

    Item {
        Timer {
            id: timerDevLoad
            interval: 2000
            running: deviceLoadDlg.visible
            repeat: false
            onTriggered: {
                deviceLoadDlg.visible = false
            }
        }
    }

    ColumnLayout {
        width: parent.width
        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: lblHeight
            verticalAlignment: Text.AlignVCenter
            font: DTK.fontManager.t5
            width: parent.height
            text: qsTr("Loading...")
        }

        RowLayout {
            spacing: 20
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.bottomMargin: 10
            Layout.topMargin: 10
            Button {
                text: qsTr("Cancel")
                Layout.preferredWidth: btnWidth
                onClicked: {
                    deviceLoadDlg.visible = false
                    timerDevLoad.stop()
                }
            }
            Button {
                text: qsTr("Ignore")
                Layout.preferredWidth: btnWidth
                Layout.alignment: Qt.AlignRight

                onClicked: {
                    deviceLoadDlg.visible = false
                    timerDevLoad.stop()
                }
            }
        }
    }
}

