// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Window 2.0
import QtQuick.Layouts 1.15
import org.deepin.dtk 1.0
import "."

/**
        Preload 作为程序的主窗口被 AppLoader 提前加载，用于达到快速
    启动的功能。 在使用时，可以配合 DWindow 的 overlayExited属性，
    进行流畅化动画设计。

        Preload 中可以使用 ApplicationWindow; Window 和 DialogWindow
    作为主窗口，但请注意，每个程序只有一个 Preload 入口，而这个 Preload 入口
    将作为程序的主界面显示，当程序存在多个 Window 窗口时，请将主窗口作为 Preload
    窗口。

        Preload 中 loadingOverlay 作为流畅化过渡的组件属性，请不要在程序加载
    完成之后使用它，也不要管理它的生命周期，其生命周期仅会在窗口过渡阶段。 loadingOverlay
    属性除了可以使用一般控件之外，也能使用静态图，动态图等内容。当不指定其大小和位置时，
    默认情况进行主窗口填充。当指定大小后，将按照控件大小和位置进行布局。
 */
ApplicationWindow {
    id: root
    visible: true
    x: Screen.width / 2 - width / 2
    y: Screen.height / 2 - height / 2
    width: 800
    height: 500
    title: qsTr("Deepin Tweak")
    flags: Qt.WindowMinButtonsHint | Qt.WindowCloseButtonHint | Qt.FramelessWindowHint
    header: TitleBar {
        leftContent: RowLayout {
            Image {
                Layout.preferredHeight: 26
                Layout.preferredWidth: 26
                Layout.leftMargin: 10
                source: "deepin-tweak.svg"
                fillMode: Image.PreserveAspectFit
            }
            Text {
                text: qsTr('Deepin Tweak')
                Layout.alignment: Qt.AlignVCenter
                font.pointSize: 24
            }
        }
        aboutDialog: DialogWindow {
            title: qsTr("Deepin Tweak")
            width: 350
            ColumnLayout {
                width: parent.width
                Item {
                    Layout.topMargin: 10
                }
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    font: DTK.fontManager.t5
                    text: qsTr("Special Thanks")
                }
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    font: DTK.fontManager.t8
                    wrapMode: "WordWrap"
                    text: qsTr("This project was developed with the great help of the deepin community. In the name of the entire community, we would like to thank all the developers and contributors who made this release possible.")
                }
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    font: DTK.fontManager.t5
                    wrapMode: "WordWrap"
                    text: qsTr("Interface Design")
                }
                Repeater {
                    model: ListModel {
                        ListElement {
                            name: "PossibleVing"
                            icon: "images/PossibleVing.png"
                        }
                        ListElement {
                            name: "deepin-community-design"
                            icon: ""
                        }
                    }
                    Row {
                        Layout.alignment: Qt.AlignHCenter
                        Image {
                            source: icon
                            width: 26
                            fillMode: Image.PreserveAspectFit
                        }
                        Label {
                            anchors.verticalCenter: parent.verticalCenter
                            font: DTK.fontManager.t8
                            wrapMode: "WordWrap"
                            text: name
                        }
                    }
                }
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    font: DTK.fontManager.t5
                    wrapMode: "WordWrap"
                    text: qsTr("Program Development")
                }
                Repeater {
                    model: ["justforlxz"]
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        font: DTK.fontManager.t8
                        wrapMode: "WordWrap"
                        text: modelData
                    }
                }
                Item {
                    Layout.bottomMargin: 10
                }
            }
        }
    }
    color: "transparent"
    background: Rectangle {
        // dark mode
        color: Qt.rgba(1, 1, 1, 0.7)
    }

    DWindow.enabled: true
    DWindow.windowRadius: 16
    DWindow.enableSystemResize: false // TODO: not disable WM max window
    DWindow.enableSystemMove: true // TODO: not working
    DWindow.enableBlurWindow: true // TODO: not working
    DWindow.loadingOverlay: Rectangle {
        color: palette.window
        BusyIndicator {
            id: indicator
            anchors.centerIn: parent
            running: true
            width: 64
            height: 64
        }
    }

    DWindow.overlayExited: Transition {
        NumberAnimation {
            properties: "scale"
            from: 1
            to: 0
            easing.type: Easing.InBack
        }
    }
}
