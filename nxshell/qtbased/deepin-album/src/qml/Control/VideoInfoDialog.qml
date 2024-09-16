// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0
import "../PreviewImageViewer/InformationDialog"
//DialogWindow {

//}

DialogWindow {

    width: 280
    property int leftX: 20
    property int topY: 70
    x: window.x+window.width - width - leftX
    y: window.y + topY
    minimumWidth: 280
    maximumWidth: 280
    minimumHeight: contentHeight4.height+60
    maximumHeight: contentHeight4.height+60

    visible: false

    property string filePath : ""

    property string fileName: FileControl.slotGetFileNameSuffix(filePath)

    header: DialogTitleBar {
        enableInWindowBlendBlur: true
        content: Loader {
            sourceComponent: Label {
                anchors.centerIn: parent
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: DTK.fontManager.t8
                text: title
                elide: Text.ElideMiddle
            }
            property string title: fileName
        }
    }

    ColumnLayout {
        id: contentHeight4
        width: 260
        anchors {
            horizontalCenter: parent.horizontalCenter
            margins: 10
        }
//        Image {
//            source: "qrc:/assets/popup/nointeractive.svg"
//        }
        PropertyItem {
            title: qsTr("Basic info")
            ColumnLayout {
                spacing: 1
                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: qsTr("File name")
                    description: fileName
                    iconName: "action_edit"
                    onClicked: {

                    }
                    corners: RoundRectangle.TopCorner
                }
                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: qsTr("Date captured")
                    description: albumControl.getMovieInfo("DateTimeOriginal",filePath)
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        title: qsTr("Size")
                        description: albumControl.getMovieInfo("Size",filePath)
                        corners: RoundRectangle.BottomLeftCorner
                    }
                    PropertyItemDelegate {
                        title: qsTr("Duration")
                        description: albumControl.getMovieInfo("Duration",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        title: qsTr("Type")
                        description: FileControl.slotFileSuffix(filePath,false)
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }
            ColumnLayout {
                spacing: 1
                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: qsTr("Path")
                    description: albumControl.getMovieInfo("Path",filePath)
                    corners: RoundRectangle.TopCorner | RoundRectangle.BottomCorner
                }
            }
        }
        PropertyItem {
            title: qsTr("Codec info")
            ColumnLayout {
                spacing: 1
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Video CodecID")
                        description: albumControl.getMovieInfo("Video CodecID",filePath)
                        corners: RoundRectangle.TopLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Video CodeRate")
                        description: albumControl.getMovieInfo("Video CodeRate",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("FPS")
                        description: albumControl.getMovieInfo("FPS",filePath)
                        corners: RoundRectangle.TopRightCorner
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth: 66
                        title: qsTr("Proportion")
                        description: albumControl.getMovieInfo("Proportion",filePath)
                        corners: RoundRectangle.BottomLeftCorner

                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth: 106
                        title: qsTr("Resolution")
                        description: albumControl.getMovieInfo("Resolution",filePath)
                        Layout.fillWidth: true
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }
        }

        PropertyItem {
            title: qsTr("Video CodecID")
            ColumnLayout {
                spacing: 1
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Audio CodecID")
                        description: albumControl.getMovieInfo("Audio CodecID",filePath)
                        corners: RoundRectangle.TopLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Audio CodeRate")
                        description: albumControl.getMovieInfo("Audio CodeRate",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("Audio digit")
                        description: albumControl.getMovieInfo("Audio digit",filePath)
                        corners: RoundRectangle.TopRightCorner
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth: 66
                        title: qsTr("Channels")
                        description: albumControl.getMovieInfo("Channels",filePath)
                        corners: RoundRectangle.BottomLeftCorner

                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth: 106
                        title: qsTr("Sampling")
                        description: albumControl.getMovieInfo("Sampling",filePath)
                        Layout.fillWidth: true
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }
        }
    }
    onVisibleChanged: {
        setX(window.x + window.width / 2 - width / 2)
        setY(window.y + window.height / 2 - height / 2)
    }
}
