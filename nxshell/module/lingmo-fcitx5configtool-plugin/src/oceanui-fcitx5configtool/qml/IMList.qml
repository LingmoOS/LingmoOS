// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import org.lingmo.dtk 1.0 as D
import org.lingmo.oceanui 1.0

Rectangle {
    id: root

    color: "transparent"
    implicitHeight: layoutView.height
    Layout.fillWidth: true

    ColumnLayout {
        id: layoutView
        width: parent.width
        clip: true
        spacing: 0

        Repeater {
            id: repeater
            model: oceanuiData.imlistModel()
            delegate: D.ItemDelegate {
                Layout.fillWidth: true
                visible: true
                cascadeSelected: true
                checkable: false
                contentFlow: true
                hoverEnabled: true
                corners: getCornersForBackground(index,
                                                 oceanuiData.imlistModel().count())

                content: RowLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    OceanUILabel {
                        Layout.fillWidth: true
                        text: model.name
                    }

                    D.ToolButton {
                        id: imManageButton
                        icon.name: "oceanui_immanage"
                        onClicked: imMenu.popup()

                        D.Menu {
                            id: imMenu

                            D.MenuItem {
                                text: qsTr("Move Up")
                                enabled: oceanuiData.imlistModel().canMoveUp(index)
                                onTriggered: oceanuiData.imlistModel().moveItem(
                                                 index, index - 1)
                            }

                            D.MenuItem {
                                text: qsTr("Move Down")
                                enabled: oceanuiData.imlistModel(
                                             ).canMoveDown(index)
                                onTriggered: oceanuiData.imlistModel().moveItem(
                                                 index, index + 1)
                            }

                            D.MenuItem {
                                text: qsTr("Settings")
                                onTriggered: oceanuiData.showIMSettingsDialog(index)
                            }

                            D.MenuSeparator {}

                            D.MenuItem {
                                text: qsTr("Remove")
                                enabled: oceanuiData.imlistModel().canRemove()
                                onTriggered: oceanuiData.imlistModel(
                                                 ).removeItem(index)
                            }
                        }
                    }
                }
                background: OceanUIItemBackground {
                    separatorVisible: true
                    backgroundType: OceanUIObject.Normal | OceanUIObject.Hover
                }
            }
        }
    }
}
