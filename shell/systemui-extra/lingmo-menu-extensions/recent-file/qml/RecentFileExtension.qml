/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import org.lingmo.menu.extension 1.0
import AppControls2 1.0 as AppControls2

import org.lingmo.quick.items 1.0 as LingmoUIItems
import org.lingmo.quick.platform 1.0 as Platform

LingmoUIMenuExtension {
    property bool isVisible: mainWindow.visible

    Component.onCompleted: {
        recentFileView.model = extensionData.recentFilesModel
        extensionData.recentFilesModel.updateData()
    }

    onIsVisibleChanged: {
        if (isVisible) {
            extensionData.recentFilesModel.updateData();
        }
    }

    Item {
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onContainsMouseChanged: {
                if (containsMouse) {
                    scrollBar.visible = true
                }
                else {
                    scrollBar.visible = false
                }
            }

            onClicked: {
                var data = {};
                data["action"] = mouse.button === Qt.LeftButton ? "closeMenu" : "emptyAreaContextMenu"
                send(data);
            }

            LingmoUIItems.StyleBackground {
                anchors.top: parent.top
                width: parent.width; height: 1
                useStyleTransparency: false
                alpha: 0.15
                paletteRole: Platform.Theme.Text
                visible: recentFileView.contentY > 0
                z: 1
            }

            ListView {
                id: recentFileView
                anchors.fill: parent
                anchors.leftMargin: 12
                spacing: 4
                focus: true
                highlightMoveDuration: 0
                onActiveFocusChanged: {
                    if (activeFocus) {
                        currentIndex = 0;
                    } else {
                        currentIndex = -1;
                    }
                }
                onCountChanged: currentIndex = -1
                keyNavigationWraps: true
                boundsBehavior: Flickable.StopAtBounds

                ScrollBar.vertical: AppControls2.ScrollBar {
                    id: scrollBar
                    visible: false
                    width: 14
                    height: recentFileView.height
                }

                delegate: LingmoUIItems.StyleBackground {
                    id: delegateItem
                    width: ListView.view.width - 18
                    height: 40

                    useStyleTransparency: false
                    paletteRole: Platform.Theme.Text
                    alpha: itemArea.containsPress ? 0.16 : itemArea.containsMouse ? 0.08 : 0.00

                    radius: Platform.Theme.minRadius
                    focus: true

                    states: State {
                        when: index === recentFileView.currentIndex
                        PropertyChanges {
                            target: delegateItem
                            alpha: 0.65
                        }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        spacing: 4

                        LingmoUIItems.Icon {
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            Layout.alignment: Qt.AlignVCenter
                            source: model.icon
                        }

                        LingmoUIItems.StyleText {
                            id: fileText
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            text: model.name
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        LingmoUIItems.StyleText {
                            Layout.preferredWidth: contentWidth
                            Layout.fillHeight: true

                            alpha: 0.35
                            text: model.date
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }

                    ToolTip {
                        visible: itemArea.containsMouse
                        delay: 500
                        contentItem: Column {
                            width: childrenRect.width
                            height: childrenRect.height
                            LingmoUIItems.StyleText {
                                verticalAlignment: Text.AlignVCenter
                                visible: fileText.truncated
                                wrapMode: Text.WrapAnywhere
                                text: model.name
                                Component.onCompleted: {
                                    if (contentWidth > recentFileView.width) {
                                        width = recentFileView.width;
                                    }
                                }
                            }
                            LingmoUIItems.StyleText {
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: Text.WordWrap
                                text: qsTr("Path: ") + model.path
                                Component.onCompleted: {
                                    if (contentWidth > recentFileView.width) {
                                        width = recentFileView.width;
                                    }
                                }
                            }
                        }
                    }

                    MouseArea {
                        id: itemArea
                        property bool hovered
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: {
                            hovered = true
                        }
                        onExited: {
                            hovered = false
                        }
                        onClicked: {
                            var data = {
                                "url": model.uri,
                                "index": model.index
                            }
                            data["action"] = mouse.button === Qt.RightButton ? "contextMenu" : "openFile"
                            send(data)
                        }
                    }
                }
            }
        }
    }
}
