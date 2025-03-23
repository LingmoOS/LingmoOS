// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Window 2.11
import QtQuick.Layouts 1.11
import org.deepin.dtk.impl 1.0 as D
import org.deepin.dtk.style 1.0 as DS

Control {
    id: control
    z: D.DTK.TopOrder
    width: Window.window.width
    // it's binding `height` instead of `visible` property,
    // because MouseArea should accept event keeping visible.
    implicitHeight: (!__isFullScreen || __isVisible) ? DS.Style.titleBar.height : 0

    property string title: Window.window.title
    property alias icon: iconLabel

    // custom control
    property alias leftContent: customLeft.sourceComponent
    property alias content: customCenter.sourceComponent

    // menu access
    property alias menu: optionMenuLoader.sourceComponent
    property bool menuDisabled
    property Component aboutDialog

    // visibility access
    property bool fullScreenButtonVisible: true
    signal toggleWindowState()
    property alias windowButtonGroup: windowButtonsLoader.sourceComponent

    property bool autoHideOnFullscreen: false
    property bool embedMode: !__dwindow.enabled
    property bool separatorVisible: true

    property var __dwindow: Window.window.D.DWindow
    property bool __isFullScreen: Window.window.visibility === Window.FullScreen
    property bool __isVisible: mouseArea.containsMouse
    readonly property int __includedAreaX: control.width - optionMenuBtn.width - windowButtonsLoader.width

    property alias enableInWindowBlendBlur: background.active

    property D.Palette textColor: DS.Style.button.text
    palette.windowText: D.ColorSelector.textColor
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: __isFullScreen && autoHideOnFullscreen
        acceptedButtons: Qt.AllButtons
        propagateComposedEvents: true

        onPressed: {
            if (mouse.button === Qt.RightButton) {
                if (mouse.x < __includedAreaX) {
                    __dwindow.popupSystemWindowMenu()
                    mouse.accepted = true
                    return
                }
            }
            mouse.accepted = false
        }
        onDoubleClicked: {
            // Windowed or Maximized
            if (mouse.button === Qt.LeftButton) {
                control.toggleWindowState()
                mouse.accepted = true
                return
            }
            mouse.accepted = false
        }
        onReleased: mouse.accepted = false
        onClicked: mouse.accepted = false
    }

    Loader {
        id: background
        active: false
        anchors.fill: parent
        sourceComponent: D.InWindowBlur {
            radius: 30
        }
    }

    ColumnLayout {
        id: content
        spacing: 0
        anchors.fill: parent
        visible: control.height > 0

        Loader {
            active: embedMode
            visible: embedMode
            sourceComponent: Rectangle {
                Layout.preferredWidth: control.width
                Layout.preferredHeight: 1
                visible: embedMode
                color: "transparent"
            }
        }

        RowLayout {
            spacing: 0
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.leftMargin: DS.Style.titleBar.leftMargin

            D.DciIcon {
                id: iconLabel
                sourceSize {
                    width: DS.Style.titleBar.iconSize
                    height: DS.Style.titleBar.iconSize
                }
                Layout.alignment: Qt.AlignLeft
                Layout.leftMargin: 2
                visible: name
                palette: D.DTK.makeIconPalette(control.palette)
                mode: control.D.ColorSelector.controlState
                theme: control.D.ColorSelector.controlTheme
            }

            // left custom area
            Loader {
                id: customLeft
            }

            // center custom area
            Loader {
                id: customCenter
                Layout.leftMargin: (optionMenuBtn.width + windowButtonsLoader.width - iconLabel.width - customLeft.width)
                Layout.fillWidth: true
                sourceComponent: titleCenterCom
            }

            // option button, it maybe load other Button in different scene.
            WindowButton {
                id: optionMenuBtn
                icon.name: "window_menu"
                textColor: control.textColor
                enabled: !menuDisabled
                Layout.alignment: Qt.AlignRight
                onClicked: {
                    if (optionMenuLoader.item && optionMenuLoader.item.visible) {
                        optionMenuLoader.item.close()
                    } else {
                        var pos = Qt.point(optionMenuLoader.x, control.height)
                        optionMenuLoader.item.popup(optionMenuBtn, pos)
                    }
                }

                Loader {
                    id: optionMenuLoader
                    sourceComponent: Menu {
                        ThemeMenu { }
                        MenuSeparator { }
                        HelpAction { }
                        AboutAction { aboutDialog: control.aboutDialog }
                        QuitAction { }
                    }
                    onLoaded: item.closePolicy = item.closePolicy | Popup.CloseOnPressOutsideParent
                }
            }

            Loader {
                id: windowButtonsLoader
                Layout.fillHeight: true
                sourceComponent: WindowButtonGroup {
                    Layout.alignment: Qt.AlignRight
                    Layout.fillHeight: true
                    embedMode: control.embedMode
                    textColor: control.textColor
                    fullScreenButtonVisible: control.fullScreenButtonVisible
                    Component.onCompleted: {
                        control.toggleWindowState.connect(maxOrWinded)
                    }
                }
            }
        }

        Loader {
            active: separatorVisible
            visible: active

            sourceComponent: Rectangle {
                Layout.preferredWidth: control.width
                Layout.preferredHeight: 1
                color: "transparent"
                Layout.alignment: Qt.AlignBottom
            }
        }
    }

    Component {
        id: titleCenterCom
        Loader {
            property bool hasWindowFlag/*: (Window.window.flags & Qt.WindowTitleHint)*/
            Component.onCompleted: hasWindowFlag = (Window.window.flags & Qt.WindowTitleHint)
            active: hasWindowFlag && !embedMode &&
                        (__dwindow.motifDecorations & D.WindowManagerHelper.DECOR_TITLE)
            visible: active

            sourceComponent: Label {
                textFormat: Text.PlainText
                text: control.title
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
