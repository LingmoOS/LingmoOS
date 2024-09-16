// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.dtk 1.0
import org.deepin.dtk.style 1.0 as DS
import org.deepin.image.viewer 1.0 as IV

Rectangle {
    property bool animationShow: true
    property string iconName: "deepin-image-viewer"
    // 当前图片是否缩放到需要隐藏标题栏
    property bool imageScaledToTitle: false

    anchors.top: window.top
    height: IV.GStatus.titleHeight
    visible: !window.isFullScreen
    width: window.width

    // 用于铺满图片时的渐变背景
    gradient: Gradient {
        GradientStop {
            color: imageScaledToTitle ? titlecontrol.ColorSelector.backgroundColor1 : titlecontrol.ColorSelector.nonGradientColor
            position: 0.0
        }

        GradientStop {
            color: imageScaledToTitle ? titlecontrol.ColorSelector.backgroundColor2 : titlecontrol.ColorSelector.nonGradientColor
            position: 1.0
        }
    }
    Behavior on y {
        enabled: visible

        NumberAnimation {
            duration: 366
            easing.type: Easing.OutExpo
        }
    }

    onAnimationShowChanged: {
        if (animationShow) {
            y = 0;
        } else {
            animationQuitDelay.restart();
        }
    }

    Timer {
        id: animationQuitDelay

        interval: 500

        onTriggered: {
            y = animationShow ? 0 : -IV.GStatus.titleHeight;
        }
    }

    // 底部阴影
    BoxShadow {
        // 调整阴影显示范围,暗色模式下仅显示底部,无扩散阴影
        property bool outterShadow: (DTK.themeType !== ApplicationHelper.DarkType)

        anchors.fill: parent
        hollow: true
        shadowBlur: outterShadow ? 10 : 1
        shadowColor: titlecontrol.ColorSelector.shadowColor
        shadowOffsetY: outterShadow ? 4 : 1
        visible: !imageScaledToTitle
    }

    BoxInsetShadow {
    }

    Control {
        id: titlecontrol

        // 渐变背景色
        property Palette backgroundColor1: Palette {
            normal: Qt.rgba(0, 0, 0, 0.15)
            normalDark: Qt.rgba(26 / 255, 26 / 255, 26 / 255, 0.6)
        }
        property Palette backgroundColor2: Palette {
            normal: Qt.rgba(0, 0, 0, 0.0)
            normalDark: Qt.rgba(26 / 255, 26 / 255, 26 / 255, 0.02)
        }
        // 非渐变背景色 BugFix: 旋转图片时不希望标题栏有透视，调整颜色为无透明度版本
        property Palette nonGradientColor: Palette {
            normal: Qt.rgba(254 / 255, 254 / 255, 254 / 255, 1)
            normalDark: Qt.rgba(32 / 255, 32 / 255, 32 / 255, 1)
        }
        // 阴影颜色
        property Palette shadowColor: Palette {
            normal: Qt.rgba(0, 0, 0, 0.03)
            normalDark: Qt.rgba(0, 0, 0, 0.9)
        }

        hoverEnabled: true // 开启 Hover 属性
    }

    IconLabel {
        anchors.left: parent.left
        anchors.leftMargin: IV.GStatus.actionMargin
        anchors.top: parent.top
        anchors.topMargin: IV.GStatus.actionMargin

        icon {
            height: 32
            name: iconName
            width: 32
        }
    }

    TitleBar {
        id: titlebar

        property Palette defaultTextColor: Palette {
            normal: Qt.rgba(0, 0, 0, 0.7)
            normalDark: Qt.rgba(1, 1, 1, 0.7)
        }
        property bool menuPopup: false
        property Palette scaledTextColor: Palette {
            normal: Qt.rgba(1, 1, 1, 0.7)
            normalDark: Qt.rgba(1, 1, 1, 0.7)
        }
        property Palette scaledTitleTextColor: Palette {
            normal: Qt.rgba(1, 1, 1, 1)
            normalDark: Qt.rgba(1, 1, 1, 1)
        }

        anchors.fill: parent
        textColor: imageScaledToTitle ? scaledTextColor : defaultTextColor

        // 使用自定的文本
        content: Loader {
            active: true

            sourceComponent: Label {
                color: imageScaledToTitle ? titlebar.ColorSelector.scaledTitleTextColor : palette.text
                // 自动隐藏多余文本
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                text: titlebar.title
                textFormat: Text.PlainText
                verticalAlignment: Text.AlignVCenter
            }
        }
        menu: Menu {
            onVisibleChanged: {
                titlebar.menuPopup = visible;
                IV.GStatus.animationBlock = visible;
            }

            // 打开图片动作项
            Action {
                id: openImageAction

                text: qsTr("Open image")

                onTriggered: {
                    // 发送打开窗口信号
                    stackView.openImageDialog();
                }
            }

            MenuSeparator {
            }

            ThemeMenu {
            }

            MenuSeparator {
            }

            HelpAction {
            }

            AboutAction {
                aboutDialog: AboutDialog {
                    description: qsTr("Image Viewer is an image viewing tool with fashion interface and smooth performance.")
                    productIcon: "deepin-image-viewer"
                    productName: qsTr("Image Viewer")
                    version: Qt.application.version
                    websiteLink: "www.chinauos.com"
                    websiteName: "www.chinauos.com"
                }
            }

            QuitAction {
            }
        }

        // 标题栏在hover，菜单展开时屏蔽动画效果，包括点击标题栏拖动
        onHoveredChanged: {
            IV.GStatus.animationBlock = hovered || menuPopup;
        }
    }
}
