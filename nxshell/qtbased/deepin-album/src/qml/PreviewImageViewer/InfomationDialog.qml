// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Window 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D
import org.deepin.dtk 1.0
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

    property string filePath: ""

    onFilePathChanged: {
        console.log("informationdialog filepath:", filePath)
    }

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
                    id: fileNameProp
                    Layout.fillWidth: true
                    title: qsTr("File name")
                    description: fileName
                    iconName: "action_edit"
                    onClicked: {

                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        title: qsTr("Size")
                        description: FileControl.slotGetInfo("FileSize",filePath)
                        corners: RoundRectangle.BottomLeftCorner
                    }
                    PropertyItemDelegate {
                        title: qsTr("Dimensions")
                        description: FileControl.slotGetInfo("Dimension",filePath)
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
                    title: qsTr("Date captured")
                    description: FileControl.slotGetInfo("DateTimeOriginal",filePath)
                    corners: RoundRectangle.TopCorner
                }

                PropertyActionItemDelegate {
                    Layout.fillWidth: true
                    title: qsTr("Date modified")
                    description: FileControl.slotGetInfo("DateTimeDigitized",filePath)
                    corners: RoundRectangle.BottomCorner
                }
            }
        }
        PropertyItem {
            title: qsTr("Details")
            ColumnLayout {
                spacing: 1
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Aperture")
                        description: FileControl.slotGetInfo("ApertureValue",filePath)
                        corners: RoundRectangle.TopLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Exposure program")
                        description: FileControl.slotGetInfo("ExposureProgram",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("Focal length")
                        description: FileControl.slotGetInfo("FocalLength",filePath)
                        corners: RoundRectangle.TopRightCorner
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth: 66
                        title: qsTr("ISO")
                        description: FileControl.slotGetInfo("ISOSpeedRatings",filePath)

                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth: 106
                        title: qsTr("Exposure mode")
                        description: FileControl.slotGetInfo("ExposureMode",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth: 86
                        title: qsTr("Exposure time")
                        description: FileControl.slotGetInfo("ExposureTime",filePath)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Flash")
                        description: FileControl.slotGetInfo("Flash",filePath)
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Flash compensation")
                        description: FileControl.slotGetInfo("FlashExposureComp",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("Max aperture")
                        description: FileControl.slotGetInfo("MaxApertureValue",filePath)
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1
                    PropertyItemDelegate {
                        contrlImplicitWidth:66
                        title: qsTr("Colorspace")
                        description: FileControl.slotGetInfo("ColorSpace",filePath)
                        corners: RoundRectangle.BottomLeftCorner
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:106
                        title: qsTr("Metering mode")
                        description: FileControl.slotGetInfo("MeteringMode",filePath)
                        Layout.fillWidth: true
                    }
                    PropertyItemDelegate {
                        contrlImplicitWidth:86
                        title: qsTr("White balance")
                        description: FileControl.slotGetInfo("WhiteBalance",filePath)
                        corners: RoundRectangle.BottomRightCorner
                    }
                }
            }

            PropertyItemDelegate {
                title: qsTr("Device model")
                description: FileControl.slotGetInfo("Model",filePath)
                corners: RoundRectangle.AllCorner
            }
            PropertyItemDelegate {
                title: qsTr("Lens model")
                description: FileControl.slotGetInfo("LensType",filePath)
                corners: RoundRectangle.AllCorner
            }
        }
    }

    onVisibleChanged: {
        setX(window.x+window.width-width-leftX)
        setY(window.y+topY)
    }

    // 窗口关闭时复位组件状态
    onClosing: {
        fileNameProp.reset();
    }
    // 图片变更时复位组件状态(切换时关闭重命名框)
    onFileNameChanged: {
        fileNameProp.reset();
    }
}
