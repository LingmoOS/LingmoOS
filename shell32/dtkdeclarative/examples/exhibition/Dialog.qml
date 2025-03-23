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

    property int dialogWidth: 400

    Flow {
        spacing: 10
        width: parent.width

        ImageCell {
            source: "qrc:/assets/dialog/Notify-Light.svg"
            DialogWindow {
                width: dialogWidth
                icon: "music"
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "名称“XXX”已被占用，请使用其他名称"
                    }
                    Button {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Layout.fillWidth: true
                        text: "确定"
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/dialog/Notify-withMSG-Light.svg"
            DialogWindow {
                width: dialogWidth
                icon: "music"
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "操作失败！"
                    }
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t7
                        text: "目标文件夹位于源文件夹内！"
                    }
                    Button {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Layout.topMargin: 10
                        Layout.fillWidth: true
                        text: "确定"
                    }
                }
            }
        }

        Item {width: parent.width; height: 1}

        ImageCell {
            source: "qrc:/assets/dialog/Authorized-Light.svg"
            DialogWindow {
                width: dialogWidth
                icon: "music"
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "XXXXXXXXXXXX需要管理员权限"
                    }
                    PasswordEdit {
                        Layout.fillWidth: true
                        text: "123456"
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Layout.topMargin: 10
                        Layout.fillWidth: true
                        Button {
                            text: "取消"
                            Layout.preferredWidth: 175
                        }
                        Item {Layout.fillWidth: true}
                        RecommandButton {
                            text: "授权"
                            Layout.preferredWidth: 175
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/dialog/Warning-Light.svg"
            DialogWindow {
                width: dialogWidth
                icon: "music"
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "格式化操作会清空该磁盘数据，您要继续吗？"
                    }
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t7
                        text: "此操作不可以恢复"
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Layout.topMargin: 10
                        Layout.fillWidth: true
                        Button {
                            text: "取消"
                            Layout.preferredWidth: 175
                        }
                        Item {Layout.fillWidth: true}
                        WarningButton {
                            text: "格式化"
                            Layout.preferredWidth: 175
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }
            }
        }

        Item {width: parent.width; height: 1}

        ImageCell {
            source: "qrc:/assets/dialog/dialog-1.svg"
            DialogWindow {
                width: dialogWidth
                icon: "music"
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "XXXXXXXXXXXX需要管理员权限"
                    }
                    ComboBox {
                        ColorSelector.family: Palette.CommonColor
                        Layout.fillWidth: true
                        model: ListModel {
                            ListElement { iconName: "entry_voice"; device: "shule"}
                            ListElement { iconName: "entry_voice"; device: "tmp"}
                        }
                        iconNameRole: "iconName"
                        textRole: "device"
                    }
                    PasswordEdit {
                        Layout.fillWidth: true
                        text: "123456"
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Button {
                            text: "取消"
                            Layout.preferredWidth: 175
                        }
                        Item {Layout.fillWidth: true}
                        RecommandButton {
                            text: "授权"
                            Layout.preferredWidth: 175
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/dialog/dialog-2.svg"
            DialogWindow {
                width: dialogWidth
                icon: "music"
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "XXXX正在准备"
                    }
                    ProgressBar {
                        Layout.fillWidth: true
                    }
                    Button {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Layout.fillWidth: true
                        text: "取消"
                    }
                }
            }
        }

        Item {width: parent.width; height: 1}

        ImageCell {
            source: "qrc:/assets/dialog/Wifi-Connection-Light.svg"
            DialogWindow {
                width: dialogWidth
                icon: "music"
                ColumnLayout {
                    width: parent.width
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "连接到隐藏网络"
                    }
                    ColumnLayout {
                        spacing: 10
                        LineEdit {
                            text: "deepin"
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            ColorSelector.family: Palette.CommonColor
                            Layout.fillWidth: true
                            model: ListModel {
                                ListElement { device: "WPA/WPA2"}
                                ListElement { device: "tmp"}
                            }
                        }
                        PasswordEdit {
                            Layout.fillWidth: true
                            text: "123456"
                        }
                    }
                    CheckBox {
                        text: "自动连接"
                        checked: true
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Button {
                            text: "取消"
                            Layout.preferredWidth: 175
                        }
                        Item {Layout.fillWidth: true}
                        Button {
                            text: "连接"
                            Layout.preferredWidth: 175
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }
            }
        }

        ImageCell {
            source: "qrc:/assets/dialog/Notify-with2Buttons-Light.svg"
            DialogWindow {
                width: dialogWidth
                minimumHeight: 120
                maximumHeight: 240
                icon: "music"
                title: showErrorDetail ? "错误信息:": ""
                property bool showErrorDetail

                ColumnLayout {
                    width: parent.width
                    Label {
                        visible: !showErrorDetail
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t5
                        text: "  这里显示简要出错信息XXXXXXXXXXX"
                    }
                    ScrollView {
                        visible: showErrorDetail
                        Layout.fillWidth: true
                        Layout.preferredHeight: 134
                        TextArea {
                            selectByMouse: true
                            wrapMode: Text.WordWrap
                            text: "VM Regions Near 0x100000002:
    -->
        __TEXT                 000000010f96f000-0000000110085000 [ 7256K] r-x/rwx SM=COW  /Applications/Adobe Illustrator CC 2018/Adobe Illustrator.app/Contents/MacOS/CEPHtmlEngine/CEPHtmlEngine.app/Contents/MacOS/CEPHtmlEngine

    Thread 0 Crashed:: CrBrowserMain  Dispatch queue: com.apple.main-thread
    0   org.chromium.ContentShell.framework	0x0000000118abe3d4 base::subtle::RefCountedThreadSafeBase::HasOneRef() const + 4
    1   org.chromium.ContentShell.framework	0x0000000118abfd4e base::internal::WeakReferenceOwner::GetRef() const + 30
    2   org.chromium.ContentShell.framework	0x000000011889b84c CefCopyFrameGenerator::OnCopyFrameCaptureCompletion(bool) + 92
    3   org.chromium.ContentShell.framework	0x000000011889badc CefCopyFrameGenerator::CopyFromCompositingSurfaceFinishedProxy(base::WeakPtr<CefCopyFrameGenerator>, std::__1::unique_ptr<cc::SingleReleaseCallback, std::__1::default_delete<cc::SingleReleaseCallback> >, gfx::Rect const&, std::__1::unique_ptr<SkBitmap, std::__1::default_delete<SkBitmap> >, std::__1::unique_ptr<SkAutoLockPixels, std::__1::default_delete<SkAutoLockPixels> >, bool) + 252
    4   org.chromium.ContentShell.framework	0x000000011889be51 void base::internal::FunctorTraits<void (*)"
                        }
                    }
                    RowLayout {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                        Layout.bottomMargin: 10
                        Button {
                            text: showErrorDetail ? "隐藏详情" : "显示详情"
                            Layout.preferredWidth: 175
                            onClicked: showErrorDetail = !showErrorDetail
                        }
                        Item {Layout.fillWidth: true}
                        Button {
                            text: "报告错误"
                            Layout.preferredWidth: 175
                        }
                    }
                }
            }
        }

        Item {width: parent.width; height: 1}

        ImageCell {
            source: "qrc:/assets/dialog/About-Light.svg"
            DialogWindow {
                width: 540
                RowLayout {
                    width: parent.width
                    ColumnLayout {
                        Layout.preferredWidth: 200
                        Layout.bottomMargin: 20
                        DciIcon {
                            Layout.alignment: Qt.AlignHCenter
                            name: "music"
                            sourceSize {
                                width: 128
                                height: 128
                            }
                        }
                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            font: DTK.fontManager.t5
                            text: "磁盘管理"
                        }

                        Image {
                            id: companyLogoLabel
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 0
                            source: "file://" + DTK.deepinDistributionOrgLogo
                        }
                        Label {
                            Layout.leftMargin: 10
                            id: websiteLabel
                            Layout.topMargin: 5
                            font: DTK.fontManager.t10
                            text: "© 1995–2021 UnionTech Inc. 保留一切权利"
                        }
                    }

                    ListView {
                        spacing: 10
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.rightMargin: 20
                        model: ListModel {
                            ListElement { title: "版本"; text: "2.1 Build 1723"}
                            ListElement { title: "主页"; text: "www.uniontech.com"}
                            ListElement { title: "描述"; text: "磁盘管理器可帮助用户进行磁盘分区、空间调整、格式化等操作。"}
                        }
                        delegate: ColumnLayout {
                            width: parent.width
                            Label {
                                font: DTK.fontManager.t10
                                text: title
                            }
                            Label {
                                text: model.text
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
        }
    }
}
