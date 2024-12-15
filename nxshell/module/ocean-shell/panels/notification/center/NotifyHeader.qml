// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.lingmo.dtk 1.0
import org.lingmo.ds.notification
import org.lingmo.ds.notificationcenter

FocusScope {
    id: root

    required property NotifyModel notifyModel
    signal headerClicked()

    // test
    onHeaderClicked: function () {
        dataPanelLoader.active = !dataPanelLoader.active
        NotifyAccessor.fetchDataInfo()
        dataPanelLoader.item.show()
    }
    Loader {
        id: dataPanelLoader
        active: false
        sourceComponent: Window {
            id: dataPanel
            width: 360
            height: 600
            x: dataPanel.transientParent.x + root.Window.width + 10
            y: dataPanel.transientParent.y
            DataPanel {
                notifyModel: root.notifyModel
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        Text {
            text: qsTr("Notification Center")
            Layout.alignment: Qt.AlignLeft
            Layout.leftMargin: 18
            font {
                pixelSize: DTK.fontManager.t4.pixelSize
                family: DTK.fontManager.t4.family
                bold: true
            }
            color: palette.windowText
            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    root.headerClicked()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
        }

        SettingActionButton {
            objectName: "collapse"
            focus: true
            visible: !notifyModel.collapse
            Layout.alignment: Qt.AlignRight
            icon.name: "fold"
            onClicked: function () {
                console.log("Collapse all notify")
                notifyModel.collapseAllApp()
            }
        }

        SettingActionButton {
            objectName: "more"
            focus: true
            Layout.alignment: Qt.AlignRight
            icon.name: "more"
            onClicked: function () {
                console.log("Notify setting")
                NotifyAccessor.openNotificationSetting()
            }
        }

        AnimationSettingButton {
            objectName: "closeAllNotify"
            icon.name: "clean-all"
            text: qsTr("Clear All")
            Layout.alignment: Qt.AlignRight
            onClicked: function () {
                console.log("Clear all notify")
                notifyModel.clear()
            }
        }
    }
}
