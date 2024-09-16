// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.image.viewer 1.0 as IV

DialogWindow {
    id: dialog

    property string fileName: IV.FileControl.slotGetFileNameSuffix(filePath)
    property url filePath: IV.GControl.currentSource
    property int leftX: 20
    property int propFullWidth: 260
    property int propLeftWidth: 66
    property int propMidWidth: 106
    property int propRightWidth: 86
    property int topY: 70

    flags: Qt.Dialog | Qt.WindowCloseButtonHint | Qt.MSWindowsFixedSizeDialogHint | Qt.WindowStaysOnTopHint
    maximumWidth: 280
    minimumWidth: 280
    visible: true
    width: 280

    header: DialogTitleBar {
        property string title: fileName

        enableInWindowBlendBlur: true

        content: Loader {
            sourceComponent: Label {
                anchors.centerIn: parent
                elide: Text.ElideMiddle
                font: DTK.fontManager.t6
                horizontalAlignment: Text.AlignHCenter
                text: title
                textFormat: Text.PlainText
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Component.onCompleted: {
        x = window.x + window.width - width - leftX;
        y = window.y + topY;
    }

    // 窗口关闭时复位组件状态
    onClosing: {
        fileNameProp.reset();
        IV.GStatus.showImageInfo = false;
    }

    // 图片变更时复位组件状态(切换时关闭重命名框)
    onFileNameChanged: {
        fileNameProp.reset();
    }

    ColumnLayout {
        id: contentHeight

        width: 260

        anchors {
            horizontalCenter: parent.horizontalCenter
            margins: 10
        }

        PropertyItem {
            title: qsTr("Basic info")

            ColumnLayout {
                spacing: 1

                PropertyActionItemDelegate {
                    id: fileNameProp

                    Layout.fillWidth: true
                    corners: RoundRectangle.TopCorner
                    description: fileName
                    iconName: "action_edit"
                    implicitWidth: propFullWidth
                    title: qsTr("File name")
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    PropertyItemDelegate {
                        contrlImplicitWidth: propLeftWidth
                        corners: RoundRectangle.BottomLeftCorner
                        description: IV.FileControl.slotGetInfo("FileSize", filePath)
                        title: qsTr("Size")
                    }

                    PropertyItemDelegate {
                        Layout.fillWidth: true
                        contrlImplicitWidth: propMidWidth
                        description: imageInfo.width + "x" + imageInfo.height
                        title: qsTr("Dimensions")
                    }

                    PropertyItemDelegate {
                        contrlImplicitWidth: propRightWidth
                        corners: RoundRectangle.BottomRightCorner
                        description: IV.FileControl.slotFileSuffix(filePath, false)
                        title: qsTr("Type")
                    }
                }
            }

            ColumnLayout {
                spacing: 1

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    corners: RoundRectangle.TopCorner
                    description: IV.FileControl.slotGetInfo("DateTimeOriginal", filePath)
                    title: qsTr("Date captured")
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    corners: RoundRectangle.BottomCorner
                    description: IV.FileControl.slotGetInfo("DateTimeDigitized", filePath)
                    title: qsTr("Date modified")
                }
            }
        }

        PropertyItem {
            id: detailInfoItem

            // 详细信息默认不显示，会影响自动布局效果，因此目前设置为固定布局
            showProperty: false
            title: qsTr("Details")

            GridLayout {
                Layout.fillWidth: true
                columnSpacing: 1
                columns: 3
                rowSpacing: 1
                rows: 4

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    corners: RoundRectangle.TopLeftCorner
                    description: IV.FileControl.slotGetInfo("ApertureValue", filePath)
                    title: qsTr("Aperture")
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    Layout.minimumWidth: propMidWidth
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("ExposureProgram", filePath)
                    title: qsTr("Exposure program")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    corners: RoundRectangle.TopRightCorner
                    description: IV.FileControl.slotGetInfo("FocalLength", filePath)
                    title: qsTr("Focal length")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    description: IV.FileControl.slotGetInfo("ISOSpeedRatings", filePath)
                    title: qsTr("ISO")
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("ExposureMode", filePath)
                    title: qsTr("Exposure mode")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    description: IV.FileControl.slotGetInfo("ExposureTime", filePath)
                    title: qsTr("Exposure time")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    description: IV.FileControl.slotGetInfo("Flash", filePath)
                    title: qsTr("Flash")
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("FlashExposureComp", filePath)
                    title: qsTr("Flash compensation")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    description: IV.FileControl.slotGetInfo("MaxApertureValue", filePath)
                    title: qsTr("Max aperture")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propLeftWidth
                    corners: RoundRectangle.BottomLeftCorner
                    description: IV.FileControl.slotGetInfo("ColorSpace", filePath)
                    title: qsTr("Colorspace")
                }

                PropertyItemDelegate {
                    Layout.fillWidth: true
                    contrlImplicitWidth: propMidWidth
                    description: IV.FileControl.slotGetInfo("MeteringMode", filePath)
                    title: qsTr("Metering mode")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propRightWidth
                    corners: RoundRectangle.BottomRightCorner
                    description: IV.FileControl.slotGetInfo("WhiteBalance", filePath)
                    title: qsTr("White balance")
                }
            }

            ColumnLayout {
                spacing: 1

                PropertyItemDelegate {
                    contrlImplicitWidth: propFullWidth
                    corners: RoundRectangle.AllCorner
                    description: IV.FileControl.slotGetInfo("Model", filePath)
                    title: qsTr("Device model")
                }

                PropertyItemDelegate {
                    contrlImplicitWidth: propFullWidth
                    corners: RoundRectangle.AllCorner
                    description: IV.FileControl.slotGetInfo("LensType", filePath)
                    title: qsTr("Lens model")
                }
            }
        }

        // 隐藏占位组件
        Item {
            id: footer

            Layout.preferredHeight: detailInfoItem.showProperty ? 5 : 10
            width: 10
        }
    }

    // DialogWindow 无法直接包含 ImageInfo
    Item {
        IV.ImageInfo {
            id: imageInfo

            frameIndex: IV.GControl.currentFrameIndex
            source: IV.GControl.currentSource
        }
    }
}
