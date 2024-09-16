// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Controls 2.4 as V2
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0 as D

D.ApplicationWindow {
    id: root
    visible: true
    width: 1200
    height: 700
    title: qsTr("dtkdeclarative")
    x:(Screen.desktopAvailableWidth - width) / 2
    y:(Screen.desktopAvailableHeight - height) / 2
//    D.DWindow.wmWindowTypes: D.WindowManagerHelper.DesktopType
    header: Example_TitleBar{}
    flags: Qt.Window | Qt.WindowMinMaxButtonsHint | Qt.WindowCloseButtonHint | Qt.WindowTitleHint
//    D.DWindow.motifFunctions: D.DWindow.motifFunctions & ~D.WindowManagerHelper.FUNC_MINIMIZE

    // 测试D.DWindow的属性
    D.DWindow.enabled: true
    D.DWindow.windowRadius: 16
    D.DWindow.borderColor: palette.highlight
    D.DWindow.borderWidth: 1
    D.DWindow.alphaBufferSize: 8

    D.FontManager {
        id: font_manager
        baseFont: font_manager.get(13, Qt.application.font)
        onFontChanged: {
            console.log("font_manager fontChanged ..........");
        }
    }

//    D.MessageManager.layout: Column {
//        anchors {
//            bottom: parent.bottom
//            right: parent.right
//        }
//    }
//    D.MessageManager.capacity: 6
//    D.MessageManager.delegate: D.FloatingMessage {
//        id: floatingMsg
//        iconName: "error"
//        type: D.FloatingMessage.TransientType
//        contentItem: V2.Label {
//            verticalAlignment: Text.AlignVCenter
//            horizontalAlignment: Text.AlignLeft
//            text: floatingMsg.content
//            elide: Text.ElideRight
//            maximumLineCount: 1
//            wrapMode: Text.Wrap
//        }

//        duration: 3000
//    }

    Component.onCompleted: {
        console.log(D.DWindow.wmWindowTypes)
        console.log(D.DWindow.enabled ? "DPlatformHandle有效" : "DPlatformHandle无效!!!!");
        console.log("hasBlurWindow : ", D.DTK.hasBlurWindow ? "true" : "false");
        console.log("windowManagerNameString : ", D.DTK.windowManagerNameString);

        // 测试系统亮暗色（枚举类型）
        if (D.DTK.themeType === D.ApplicationHelper.LightType) {
            console.log("themeType : Light Theme");
        } else if (D.DTK.themeType === D.ApplicationHelper.DarkType) {
            console.log("themeType : Dark Theme");
        } else {
            console.log("themeType : Unknown Theme");
        }
        D.DTK.themeTypeChanged.connect(function() {
            console.log("themeNameChanged : ", D.DTK.themeType);
        });

        if (D.DTK.windowManagerName === D.WindowManagerHelper.DeepinWM) {
            console.log("windowManagerName : DeepinWM");
        } else if (D.DTK.windowManagerName === D.WindowManagerHelper.KWinWM) {
            console.log("windowManagerName : KWinWM");
        } else {
            console.log("windowManagerName : OtherWM");
        }

        console.log("test platformTheme info : " , D.DTK.platformTheme.activeColor, D.DTK.platformTheme.themeName);

        //测试DPlatformThemeProxy信号传递(控制中心切换主题测试)
        D.DTK.platformTheme.themeNameChanged.connect(function(themeName) {
            console.log("platformTheme themeNameChanged : ", themeName);
        });

        //在控制中心修改字体大小可以看到打印输出
        D.DTK.fontManager.fontChanged.connect(function() {
            console.log("base font changed ....", D.DTK.fontManager.baseFont);
        });
        console.log("fontManager t1 字体信息", D.DTK.fontManager.t1);

        // 测试 D.Palette 值
        console.log("D.Palette window value: ", root.palette.window)
        console.log("D.Palette windowText value: ", root.palette.windowText)
        console.log("D.Palette base value: ", root.palette.base)
        console.log("D.Palette itemBackground value: ", root.palette.itemBackground)
        console.log("D.Palette textTitle value: ", root.palette.textTitle)
        console.log("D.Palette textTips value: ", root.palette.textTips)
    }

    V2.TabBar {
        id: tabBar
        anchors.top: root.bottom
        V2.TabButton {
            text: qsTr("Example_1")
        }
        V2.TabButton {
            text: qsTr("Example_2")
        }
        V2.TabButton {
            text: qsTr("ItemViewport")
        }
        V2.TabButton {
            text: qsTr("Config")
        }
        V2.TabButton {
            text: qsTr("SettingsDialog")
        }
        V2.TabButton {
            text: qsTr("Notify")
        }
        V2.TabButton {
            text: qsTr("ColorSelector")
        }
        V2.TabButton {
            text: qsTr("Menu")
        }
        V2.TabButton {
            text: qsTr("Popup")
        }
        V2.TabButton {
            text: qsTr("Flickable")
        }
    }
    StackLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.margins: 4

        currentIndex: tabBar.currentIndex
        Example_1 {}
        Example_2 {}
        Example_3 {}
        Example_config {}
        Example_settingsdialog {}
        Example_Notify {}
        Example_colorselector {}
        Example_Menu {}
        Example_Popup {}
        Example_Flickable {}
    }

    D.BehindWindowBlur {
        anchors {
            fill: parent
            topMargin: parent.height - 100
        }
        blendColor: Qt.rgba(1, 0, 0, 0.3)
    }

    D.StyledBehindWindowBlur {
        control: root
        anchors {
            right: parent.right
            rightMargin: 100
            bottom: parent.bottom
            bottomMargin: 160
        }
        width: 200
        height: 200
        cornerRadius: 30
        V2.Label {
            anchors.centerIn: parent
            text: qsTr("WMBlur")
        }
    }
}
