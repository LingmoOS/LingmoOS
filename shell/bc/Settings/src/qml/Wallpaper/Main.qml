import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtGraphicalEffects 1.0
import QtQuick.Dialogs 1.2
import Lingmo.Settings 1.0
import LingmoUI 1.0 as LingmoUI

import "../"

ItemPage {
    headerTitle: qsTr("Background")

    Background {
        id: background
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: LingmoUI.Units.largeSpacing

            // FileDialog {
            //     id: fileDialog
            //     folder: shortcuts.pictures
            //     nameFilters: ["Image files (*.jpg *.png)", "All files (*)"]
            //     selectFolder: fales
            //     onAccepted: {
            //         _image.sources = fileDialog.fileUrl
            //         fileDialog.baseData = fileDialog.fileUrl
            //         background.setBackground(baseData)
            //     }
            // }

            // DesktopPreview {
            //    Layout.alignment: Qt.AlignHCenter
            //    width: 500
            //    height: 300
            // }
            Rectangle {
                id: deviceItem

                Layout.alignment: Qt.AlignHCenter
                // width: logo.width/1.2
                // height: logo.width/2
                width: 350
                height: 220
                color: LingmoUI.Theme.backgroundColor
                border.width: 5
                border.color: LingmoUI.Theme.textColor
                radius: LingmoUI.Theme.bigRadius

                Image {
                    id: _image
                    width: deviceItem.width - 6
                    height: deviceItem.height - 6
                    anchors {
                        verticalCenter: parent.verticalCenter
                        horizontalCenter: parent.horizontalCenter
                    }
                    // source: LingmoUI.Theme.darkMode ? "qrc:/images/MundoDark.jpeg" : "qrc:/images/MundoLight.jpeg"
                    source: "file://" + background.currentBackgroundPath
                    sourceSize: Qt.size(width, height)
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    mipmap: true
                    cache: true
                    smooth: true
                    opacity: 1.0

                    Behavior on opacity {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutCubic
                        }
                    }

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Item {
                            width: _image.width
                            height: _image.height
                            Rectangle {
                                anchors.fill: parent
                                radius: LingmoUI.Theme.bigRadius
                            }
                        }
                    }
                }

                Rectangle {
                    id: dockItem
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: _image.bottom
                    anchors.bottomMargin: 4
                    width: 120
                    height: 20
                    color: LingmoUI.Theme.backgroundColor
                    opacity: 0.85
                    border.width: 0.1
                    border.color: LingmoUI.Theme.textColor
                    radius: LingmoUI.Theme.bigRadius/2.5
                }

                Rectangle {
                    id: icon1
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: dockItem.left
                    anchors.leftMargin: 2
                    width: dockItem.height-6
                    height: icon1.width
                    color: "#D4D4D4"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 3
                }

                Rectangle {
                    id: icon2
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon1.right
                    anchors.leftMargin: 3
                    width: dockItem.height-6
                    height: icon1.width
                    color: "#DBC81E"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 3
                }

                Rectangle {
                    id: icon3
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon2.right
                    anchors.leftMargin: 3
                    width: dockItem.height-6
                    height: icon1.width
                    color: "#464444"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 3
                }

                Rectangle {
                    id: icon4
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon3.right
                    anchors.leftMargin: 3
                    width: dockItem.height-6
                    height: icon1.width
                    color: "#4891FF"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 3
                }

                Rectangle {
                    id: icon5
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon4.right
                    anchors.leftMargin: 3
                    width: dockItem.height-6
                    height: icon1.width
                    color: "#FD5D93"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 3
                }

                Rectangle {
                    id: icon6
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon5.right
                    anchors.leftMargin: 3
                    width: dockItem.height-6
                    height: icon1.width
                    color: "#FFA08A"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 3
                }

                Rectangle {
                    id: icon7
                    // anchors.horizontalCenter: dockItem.horizontalCenter
                    anchors.verticalCenter: dockItem.verticalCenter
                    anchors.left: icon6.right
                    anchors.leftMargin: 3
                    width: dockItem.height-6
                    height: icon1.width
                    color: "#FF8AFF"
                    // opacity: 0.85
                    // border.width: 0.1
                    // border.color: LingmoUI.Theme.textColor
                    radius: 3
                }
            }

            RoundedItem {
                RowLayout {
                    spacing: LingmoUI.Units.largeSpacing * 2

                    Label {
                        text: qsTr("Background type")
                        leftPadding: LingmoUI.Units.smallSpacing
                    }

                    TabBar {
                        Layout.fillWidth: true

                        onCurrentIndexChanged: {
                            background.backgroundType = currentIndex
                        }

                        Component.onCompleted: {
                            currentIndex = background.backgroundType
                        }

                        TabButton {
                            text: qsTr("Picture")
                        }

                        TabButton {
                            text: qsTr("Color")
                        }
                    }
                }

                GridView {
                    id: _view

                    property int rowCount: _view.width / itemWidth

                    Layout.fillWidth: true
                    implicitHeight: Math.ceil(_view.count / rowCount) * cellHeight + LingmoUI.Units.largeSpacing

                    visible: background.backgroundType === 0

                    clip: true
                    model: background.backgrounds
                    currentIndex: -1
                    interactive: false

                    cellHeight: itemHeight
                    cellWidth: calcExtraSpacing(itemWidth, _view.width) + itemWidth

                    property int itemWidth: 180
                    property int itemHeight: 127

                    delegate: Item {
                        id: item

                        property bool isSelected: modelData === background.currentBackgroundPath

                        width: GridView.view.cellWidth
                        height: GridView.view.cellHeight
                        scale: 1.0

                        Behavior on scale {
                            NumberAnimation {
                                duration: 200
                                easing.type: Easing.OutSine
                            }
                        }

                        // Preload background
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: LingmoUI.Units.largeSpacing
                            radius: LingmoUI.Theme.bigRadius + LingmoUI.Units.smallSpacing / 2
                            color: LingmoUI.Theme.backgroundColor
                            visible: _image.status !== Image.Ready
                        }

                        // Preload image
                        Image {
                            anchors.centerIn: parent
                            width: 32
                            height: width
                            sourceSize: Qt.size(width, height)
                            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/picture.svg"
                                                          : "qrc:/images/light/picture.svg"
                            visible: _image.status !== Image.Ready
                        }

                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: LingmoUI.Units.smallSpacing
                            color: "transparent"
                            radius: LingmoUI.Theme.bigRadius + LingmoUI.Units.smallSpacing / 2

                            border.color: LingmoUI.Theme.highlightColor
                            border.width: _image.status == Image.Ready & isSelected ? 3 : 0

                            Image {
                                id: _image
                                anchors.fill: parent
                                anchors.margins: LingmoUI.Units.smallSpacing
                                source: "file://" + modelData
                                sourceSize: Qt.size(width, height)
                                fillMode: Image.PreserveAspectCrop
                                asynchronous: true
                                mipmap: true
                                cache: true
                                smooth: true
                                opacity: 1.0

                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 100
                                        easing.type: Easing.InOutCubic
                                    }
                                }

                                layer.enabled: true
                                layer.effect: OpacityMask {
                                    maskSource: Item {
                                        width: _image.width
                                        height: _image.height

                                        Rectangle {
                                            anchors.fill: parent
                                            radius: LingmoUI.Theme.bigRadius
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton
                                hoverEnabled: true

                                onClicked: {
                                    background.setBackground(modelData)
                                }

                                onEntered: function() {
                                    _image.opacity = 0.7
                                }
                                onExited: function() {
                                    _image.opacity = 1.0
                                }

                                onPressedChanged: item.scale = pressed ? 0.97 : 1.0
                            }
                        }
                    }

                    function calcExtraSpacing(cellSize, containerSize) {
                        var availableColumns = Math.floor(containerSize / cellSize)
                        var extraSpacing = 0
                        if (availableColumns > 0) {
                            var allColumnSize = availableColumns * cellSize
                            var extraSpace = Math.max(containerSize - allColumnSize, 0)
                            extraSpacing = extraSpace / availableColumns
                        }
                        return Math.floor(extraSpacing)
                    }
                }

                Item {
                    visible: background.backgroundType === 1
                    height: LingmoUI.Units.smallSpacing
                }

                Loader {
                    Layout.fillWidth: true
                    height: item.height
                    visible: background.backgroundType === 1
                    sourceComponent: colorView
                }
            }

            Item {
                height: LingmoUI.Units.largeSpacing
            }
        }
    }

    Component {
        id: colorView

        GridView {
            id: _colorView
            Layout.fillWidth: true

            property int rowCount: _colorView.width / cellWidth

            implicitHeight: Math.ceil(_colorView.count / _colorView.rowCount) * cellHeight + LingmoUI.Units.largeSpacing

            cellWidth: 50
            cellHeight: 50

            interactive: false
            model: ListModel {}

            property var itemSize: 32

            Component.onCompleted: {
                model.append({"bgColor": "#2B8ADA"})
                model.append({"bgColor": "#4DA4ED"})
                model.append({"bgColor": "#FF5795"})
                model.append({"bgColor": "#FF8695"})
                model.append({"bgColor": "#008484"})
                model.append({"bgColor": "#B7E786"})
                model.append({"bgColor": "#F2BB73"})
                model.append({"bgColor": "#EE72EB"})
                model.append({"bgColor": "#F0905A"})
                model.append({"bgColor": "#C6C6C6"})
                model.append({"bgColor": "#595959"})
                model.append({"bgColor": "#000000"})
            }

            delegate: Rectangle {
                property bool checked: Qt.colorEqual(background.backgroundColor, bgColor)
                property color currentColor: bgColor

                width: _colorView.itemSize + LingmoUI.Units.largeSpacing
                height: width
                color: "transparent"
                radius: width / 2
                border.color: _mouseArea.pressed ? Qt.rgba(currentColor.r,
                                                           currentColor.g,
                                                           currentColor.b, 0.6)
                                                 : Qt.rgba(currentColor.r,
                                                           currentColor.g,
                                                           currentColor.b, 0.4)
                border.width: checked ? 3 : _mouseArea.containsMouse ? 2 : 0

                MouseArea {
                    id: _mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: background.backgroundColor = bgColor
                }

                Rectangle {
                    width: 32
                    height: width
                    anchors.centerIn: parent
                    color: currentColor
                    radius: width / 2
                }
            }
        }
    }
}
