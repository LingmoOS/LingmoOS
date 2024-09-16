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
    height: DS.Style.dialogWindow.titleBarHeight

    // custom control
    property alias content: customCenter.sourceComponent
    // dialog icon
    property alias icon: iconLabel
    property string title
    property alias enableInWindowBlendBlur: background.visible

    property var __dwindow: Window.window.D.DWindow

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        propagateComposedEvents: true
        onPressed: {
            if (mouse.button === Qt.RightButton) {
                if (mouse.x < control.width - closeBtn.width) {
                    __dwindow.popupSystemWindowMenu()
                    mouse.accepted = true
                    return
                }
            }
            mouse.accepted = false
        }
    }

    background: D.InWindowBlur {
        id: background
        radius: 30
        visible: false
    }

    contentItem: ColumnLayout {
        id: content
        spacing: 0

        Rectangle {
            id: separatorTop
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 1
            visible: !__dwindow.enabled
            color: "transparent"
        }

        RowLayout {
            spacing: 0
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width
            Layout.leftMargin: DS.Style.titleBar.leftMargin

            D.DciIcon {
                id: iconLabel
                visible: name !== ""
                mode: control.D.ColorSelector.controlState
                theme: control.D.ColorSelector.controlTheme
                palette: D.DTK.makeIconPalette(control.palette)
                sourceSize {
                    width: DS.Style.dialogWindow.iconSize
                    height: DS.Style.dialogWindow.iconSize
                }
            }

            // center custom area
            Loader {
                id: customCenter
                Layout.leftMargin: closeBtn.width - iconLabel.width
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true
                Layout.fillWidth: true
                sourceComponent: titleCenterCom
            }

            WindowButton {
                id: closeBtn
                icon.name: "window_close"
                property bool hasWindowFlag/*: (Window.window.flags & Qt.WindowCloseButtonHint)*/
                Component.onCompleted: hasWindowFlag = (Window.window.flags & Qt.WindowCloseButtonHint)

                visible: hasWindowFlag && __dwindow.enabled

                enabled: __dwindow.motifFunctions & D.WindowManagerHelper.FUNC_CLOSE
                onClicked: Window.window.close()
            }
        }

        Rectangle {
            id: separatorButtom
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: 1
            color: "transparent"
            Layout.alignment: Qt.AlignBottom
        }
    }

    Component {
        id: titleCenterCom
        Label {
            textFormat: Text.PlainText
            text: control.title
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
