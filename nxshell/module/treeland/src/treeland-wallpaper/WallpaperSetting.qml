// Copyright (C) 2023 pengwenhao <pengwenhao@gmail.com>.
// SPDX-License-Identifier: Apache-2.0 OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 2.15
import Qt5Compat.GraphicalEffects
import QtQuick.Dialogs
import Wallpaper
import org.lingmo.dtk 1.0

Item {
    id: root
    implicitWidth: parent.width
    implicitHeight: 168

    property alias imageSource: backgroundImage.source

    ColumnLayout {
        spacing: 10
        anchors.fill: parent

        Text {
            id: title
            Layout.alignment: Qt.AlignTop
            width: parent.width
            height: 32
            verticalAlignment: Text.AlignVCenter
            text: qsTr("Wallpaper")
        }

        Item {
            implicitHeight: root.height - title.height - parent.spacing
            implicitWidth: root.width

            RowLayout{
                spacing: 10
                implicitWidth: parent.width

                Item {
                    id: background
                    height: 135
                    width: 240
                    Layout.alignment: Qt.AlignLeft

                    Image {
                        id: backgroundImage
                        anchors.fill: parent
                        anchors.margins: 4
                        fillMode: Image.PreserveAspectCrop
                        asynchronous: true
                        sourceSize: Qt.size(width, height)
                    }

                    Rectangle {
                        id: maskRect
                        anchors.fill: parent
                        radius: 10
                    }

                    OpacityMask {
                        anchors.fill: parent
                        source: backgroundImage
                        maskSource: maskRect
                    }
                }

                Item {
                    implicitHeight: background.height
                    implicitWidth: parent.width - background.width - parent.spacing

                    ColumnLayout {
                        implicitWidth: parent.width
                        spacing: 10

                        ColumnLayout {
                            id: information
                            implicitWidth: parent.width
                            spacing: 1

                            PropertyItemDelegate {
                                implicitWidth: parent.width
                                title: qsTr("Current Wallpaepr")

                                corners: RoundRectangle.TopCorner
                                action: Text {
                                    text: personalization.currentGroup
                                }
                            }

                            PropertyItemDelegate {
                                implicitWidth: parent.width
                                title: qsTr("Screen Output")

                                action: ComboBox {
                                    ColorSelector.family: Palette.CommonColor
                                    model: personalization.outputModel()

                                    Component.onCompleted: {
                                        personalization.output = currentText
                                    }

                                    onActivated: {
                                        personalization.output = currentText
                                    }
                                }
                            }

                            PropertyItemDelegate {
                                implicitWidth: parent.width
                                title: qsTr("Wallpaper Display Method")

                                corners: RoundRectangle.BottomCorner
                                action: ComboBox {
                                    enabled: false
                                    ColorSelector.family: Palette.CommonColor
                                    model: ListModel {
                                        ListElement { text: qsTr("Stretch") }
                                        ListElement { text: qsTr("Preserve Aspect Fit") }
                                    }
                                    textRole: "text"
                                }
                            }
                        }

                        FileDialog {
                            id: fileDialog
                            options: FileDialog.DontUseNativeDialog
                            title: qsTr("Please choose a file")

                            onAccepted: {
                                personalization.addWallpaper(fileDialog.selectedFile);
                            }
                        }

                        RowLayout {
                            implicitWidth: parent.width
                            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                            spacing: 10

                            Button {
                                id: add_wallpaper
                                property var wallpaperPath : fileDialog.fileUrls
                                text: qsTr("Add Wallpaper")
                                Layout.preferredWidth: (parent.width - parent.spacing) / 2
                                Layout.alignment: Qt.AlignLeft

                                onClicked: fileDialog.open()
                            }

                            Button {
                                id: add_directory
                                text: qsTr("Add Directory")
                                visible: false
                                Layout.preferredWidth: (parent.width - parent.spacing) / 2
                                Layout.alignment: Qt.AlignRight
                            }

                            Item {
                                id: spacer
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }
    }
}
