// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

import org.lingmo.oceanui 1.0
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS
import org.lingmo.dtk.private as P

ColumnLayout {
    id: root
    property var model
    readonly property int imageRectH: 120
    readonly property int imageRectW: 180
    readonly property int imageSpacing: 10
    property bool isExpand: false
    property var currentItem

    signal wallpaperSelected(var url, bool isDark, bool isLock)

    onIsExpandChanged: {
        sortedModel.update()
    }

    RowLayout {
        id: titleLayout
        Layout.preferredHeight: 50
        Layout.fillWidth: true
        Label {
            Layout.leftMargin: 10
            font: D.DTK.fontManager.t4
            text: oceanuiObj.displayName
        }
        Item {
            Layout.fillWidth: true
        }
        D.ToolButton {
            text: isExpand ? qsTr("unfold") : qsTr("show all") + `-${model.rowCount()}` + qsTr("items")
            font: D.DTK.fontManager.t6
            flat: true
            onClicked: {
                isExpand = !isExpand
            }
        }
    }

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: containter.height

        // animation used
        Rectangle {
            id: containter
            width: parent.width
            height: layout.height
            color: "transparent"
            Behavior on height {
                NumberAnimation {
                    duration: 300
                    easing.type: Easing.OutQuart
                }
            }
        }

        Flow {
            id: layout
            property int lineCount: Math.floor((parent.width + imageSpacing) / (imageRectW + imageSpacing))
            width: lineCount * (imageRectW + imageSpacing) - imageSpacing
            spacing: imageSpacing
            bottomPadding: imageSpacing
            anchors.horizontalCenter: parent.horizontalCenter

            move: Transition {
            }

            Repeater {
                model: sortedModel
            }
        }

        D.SortFilterModel {
            id: sortedModel
            model: root.model
            property int maxCount: layout.lineCount * 2
            lessThan: function(left, right) {
                return left.index < right.index
            }
            filterAcceptsItem: function(item) {
                return isExpand ? true : item.index < maxCount
            }
            onMaxCountChanged: {
                sortedModel.update()
            }

            delegate: Control {
                id: control
                width: root.imageRectW
                height: root.imageRectH
                hoverEnabled: true

                contentItem: Item {
                    id: wallpaperItem
                    readonly property int imageMargin: 3
                    function requestSetWallpaper(isLock) {
                        img2x2.grabToImage(function(result) {
                            const isDarkType = oceanuiData.imageHelper.isDarkType(result.image);
                            root.wallpaperSelected(model.url, isDarkType, isLock)
                        }, Qt.size(2, 2));
                    }

                    Image {
                        property bool isDarktype: true
                        anchors.centerIn : parent
                        id: img2x2
                        width: 2
                        height: 2
                        source: model.url
                        fillMode: Image.Stretch
                        asynchronous: true
                    }

                    Image {
                        anchors.fill: parent
                        anchors.margins: parent.imageMargin
                        id: image
                        source: model.url
                        sourceSize: Qt.size(image.width, image.height)
                        visible: false
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                    }

                    Rectangle {
                        anchors.fill: parent
                        visible: model.url === root.currentItem && model.url !== undefined
                        color: "transparent"
                        border.width: 2
                        border.color: D.DTK.platformTheme.activeColor
                        radius: 9
                    }

                    OpacityMask {
                        anchors.fill: parent
                        anchors.margins: parent.imageMargin
                        source: image
                        maskSource: Rectangle {
                            implicitWidth: image.width
                            implicitHeight: image.height
                            radius: 8
                        }
                    }
                    Control {
                        implicitHeight: 24
                        implicitWidth: 24
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.topMargin: - height / 2 + parent.imageMargin
                        anchors.rightMargin: - width / 2 + parent.imageMargin
                        hoverEnabled: true
                        contentItem: D.IconButton {
                            icon.name: "close"
                            // visible: control.hovered || parent.hovered
                            // FIXME: force false
                            visible: false
                            background: P.ButtonPanel {
                                implicitWidth: DS.Style.iconButton.backgroundSize
                                implicitHeight: DS.Style.iconButton.backgroundSize
                                radius: width / 2
                                button: control
                            }
                            scale: visible ? 1 : 0
                            Behavior on scale {
                                NumberAnimation {
                                    duration: 300
                                    easing.type: Easing.OutExpo
                                }
                            }
                        }
                    }


                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: {
                            if (mouse.button === Qt.LeftButton) {
                                wallpaperItem.requestSetWallpaper(false)
                            } else if (mouse.button === Qt.RightButton) {
                                contextMenu.x = mouse.x
                                contextMenu.y = mouse.y
                                contextMenu.open()
                            }
                        }
                    }

                    D.Menu {
                        id: contextMenu
                        MenuItem {
                            text: qsTr("Set lock screen")
                            onTriggered: {
                                wallpaperItem.requestSetWallpaper(false)
                            }
                        }
                        MenuItem {
                            text: qsTr("Set all")
                            onTriggered: {
                                wallpaperItem.requestSetWallpaper(true)
                                wallpaperItem.requestSetWallpaper(false)
                            }
                        }
                    }
                }
            }
        }
    }
}