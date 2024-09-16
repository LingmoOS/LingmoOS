// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import org.deepin.dtk 1.0
import ".."

Column {
    id: control
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "通知类控件， 包括系统通知、应用内通知、角标等形式。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    Flow {
        spacing: 10
        width: parent.width

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-SimpleInfo-Light-1.png"
            onClicked: DTK.sendSystemMessage("相册", "已成功导入825张照片到相册", "music")
        }

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-SimpleInfo-Light-3.png"
            onClicked: DTK.sendSystemMessage("相册", "已成功导入825张照片到相册", "music", ["_action", "关闭"])
        }

        Item { width: parent.width; height: 1}

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-SimpleInfo-Light-2.png"
            onClicked: DTK.sendSystemMessage("解压缩", "压缩文件“XXXXXXXX.zip”解压失败！", "music")
        }

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-SimpleInfo-Light-4.png"
            onClicked: DTK.sendSystemMessage("解压缩", "压缩文件“XXXXXXXX.zip”解压失败！", "music", ["_action", "关闭"])
        }

        Item { width: parent.width; height: 1}

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-MultiInfo-Light-5.png"
            onClicked: DTK.sendSystemMessage("舒乐", "大家好，V23设计已经进行了一段时间，附件里包含部分V23设计图，简要向大家汇报一下", "music")
        }

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-MultiInfo-Light-7.png"
            onClicked: DTK.sendSystemMessage("舒乐", "大家好，V23设计已经进行了一段时间，附件里包含部分V23设计图，简要向大家汇报一下", "music", ["_action", "回复", "_select", "已读"])
        }

        Item { width: parent.width; height: 1}

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-MultiInfo-Light-6.png"
            onClicked: DTK.sendSystemMessage("舒乐", "Copyright 2014-2019 Adobe (http://www.adobe.com/), with Reserved FontName 'Source'. Source is a trademark of Adobe in the United States and/or other countries."
                                             , "music")
        }

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Temporary-MultiInfo-Light-8.png"
            onClicked: DTK.sendSystemMessage("舒乐", "Copyright 2014-2019 Adobe (http://www.adobe.com/), with Reserved FontName 'Source'. Source is a trademark of Adobe in the United States and/or other countries."
                                                           , "music", ["_action", "回复", "_select", "已读"])
        }

        Item { width: parent.width; height: 1}

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Interactive-SimpleInfo-Light-9.png"
            onClicked: DTK.sendSystemMessage("相册", "发送文件“UOS桌面操作系统使用说明书.pdf ”等3个文件给您，是否接收？"
                                                           , "music", ["_action", "拒绝", "_select", "接收"])
        }

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Interactive-SimpleInfo-Light-11.png"
            onClicked: DTK.sendSystemMessage("相册", "发送文件“UOS桌面操作系统使用说明书.pdf ”等3个文件给您，是否接收？"
                                                           , "music", ["_action", "拒绝", "_select", "接收"])
        }

        Item { width: parent.width; height: 1}

        ImageCell {
            imageScale: 0.5
            source: "qrc:/assets/system-notification/Interactive-SimpleInfo-Light-10.png"
            onClicked: DTK.sendSystemMessage("相册", "已成功导入825张照片到相册", "music")
        }

        ImageCell {
            source: "qrc:/assets/popup/nointeractive.svg"
            onClicked: {
                DTK.sendMessage(control, "成功添加到＂校园名谣＂", "checked")
            }
        }

        ImageCell {
            source: "qrc:/assets/popup/interactive.svg"
            onClicked: DTK.sendMessage(Window.window, floatingMsgCom, {
                                           content: "磁盘中的原文件已被修改，是否重新载入？",
                                           iconName: "music"
                                       }, -1)
        }

        Component {
            id: floatingMsgCom
            FloatingMessage {
                id: floatMsg
                contentItem: RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Label {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        text: floatMsg.message.content
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                        maximumLineCount: 1
                        font: DTK.fontManager.t6
                    }

                    Button {
                        text: "重新载入"
                        font: DTK.fontManager.t6
                        Layout.alignment: Qt.AlignVCenter
                        Layout.preferredWidth: 92
                        Layout.preferredHeight: 28
                    }
                }
            }
        }

        Item {width: parent.width; height: 1}

        Row {
            spacing: 150
            Loader {
                sourceComponent: appBadgeCom
                property int number: 1
                property string appName: "deepin-app-store"
            }
            Loader {
                sourceComponent: appBadgeCom
                property int number: 99
                property string appName: "deepin-browser"
            }
            Loader {
                sourceComponent: appBadgeCom
                property int number: 100
                property string appName: "deepin-editor"
            }
            Loader {
                sourceComponent: appBadgeCom
                property int number: 1000
                property string appName: "deepin-diskmanager"
            }
            Component {
                id: appBadgeCom

                QtIcon {
                    name: appName
                    sourceSize {
                        width: 128
                        height: width
                    }
                    Label {
                        height: 20
                        width: Math.max(20, implicitWidth)
                        z: DTK.AboveOrder
                        anchors {
                            right: parent.right
                            rightMargin: 4
                            top: parent.top
                            topMargin: 4
                        }
                        topPadding: 5
                        rightPadding: 7
                        leftPadding: 7
                        text: {
                            if (number >= 0 && number < 100) {
                                return number
                            } else if (number >= 100 && number < 1000) {
                                return number + "+"
                            }
                            return "!"
                        }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font: DTK.fontManager.t5
                        color: palette.window
                        background: Rectangle {
                            implicitHeight: 20
                            implicitWidth: 20
                            radius: 10
                            BoxShadow {
                                anchors.fill: parent
                                shadowBlur: 3
                                shadowOffsetY: 2
                                shadowColor: Qt.rgba(255, 113, 113, 0.3)
                                cornerRadius: 0
                            }
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#ff674a" }
                                GradientStop { position: 1.0; color: "#ec5783" }
                            }
                        }
                    }
                }
            }
        }

        Item {width: parent.width; height: 1}
    }
}
