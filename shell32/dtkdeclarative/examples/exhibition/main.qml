// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.4
// 确保在最后引入
import org.deepin.dtk 1.0

ApplicationWindow {
    id: root
    visible: true
    width: Math.max(contentList.contentWidth + navigation.width, 1500)
    height: 800
    title: qsTr("DTK Exhibition(Scene Graph Backend: %1)").arg(globalObject.sceneGraphBackend(root))

    // 开启“圆角窗口&无系统标题栏”模式
    DWindow.enabled: true

    ButtonGroup {
        id: activeColorSelector
        onCheckedButtonChanged: {
            root.palette.highlight = checkedButton.color
        }
    }

    header: TitleBar {
        enableInWindowBlendBlur: true
        content: Item {
            SearchEdit {
                anchors.centerIn: parent
                width: 300
            }
        }
        menu: Menu {
            Action {
                text: "Light Theme"
                onTriggered: {
                    ApplicationHelper.setPaletteType(ApplicationHelper.LightType)
                }
            }
            Action {
                text: "Dark Theme"
                onTriggered: {
                    ApplicationHelper.setPaletteType(ApplicationHelper.DarkType)
                }
            }
            MenuItem {
                contentItem: Item {
                    Row {
                        anchors.centerIn: parent
                        Repeater {
                            model: ["#d8316c", "#ff5d00", "#f8cb00", "#23c400", "#00a48a", "#0081ff", "#3c02d7", "#8c00d4"]
                            delegate: ColorButton {
                                color: modelData
                                Component.onCompleted: {
                                    activeColorSelector.addButton(this)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: sourceViewerDialog
        width: parent.width
        height: parent.height
        modal: true

        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0 }
        }

        Overlay.modal: Rectangle {
            color: palette.window
        }

        ViewQMLSource {
            id: sourceViewer
            anchors.fill: parent
        }
    }

    RowLayout {
        anchors.fill: parent
        ListView {
            id: navigation
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            Layout.margins: 10
            model: examplesFiles
            delegate: ItemDelegate {
                text: modelData.substring(0, modelData.indexOf(".qml"))
                backgroundVisible: false
                onClicked: navigation.currentIndex = index
                checked: navigation.currentIndex === index
            }
            currentIndex: 0
            onCurrentIndexChanged: {
                contentList.currentIndex = currentIndex
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            padding: 10
            ListView {
                id: contentList
                spacing: 10
                model: examplesFiles
                leftMargin: 10
                rightMargin: 10
                clip: true
                delegate: Column {
                    width: contentList.width - contentList.leftMargin - contentList.rightMargin
                    Loader {
                        source: "qrc:/examples/" + modelData
                        width: parent.width
                        asynchronous: false
                    }
                }
                currentIndex: 0
                onCurrentIndexChanged: {
                    navigation.currentIndex = currentIndex
                }
                section {
                    property: "modelData"
                    delegate: GroupTitle {
                        text: section
                        width: contentList.width - contentList.leftMargin - contentList.rightMargin
                        onViewSource: {
                            sourceViewer.url = "qrc:/examples/" + section
                            sourceViewerDialog.open()
                        }
                    }
                }
            }
            background: Rectangle {
                implicitWidth: 600
                implicitHeight: 800
                color:  Qt.rgba(0, 0, 0, 0.3)
                Rectangle {
                    anchors {
                        fill: parent
                        margins: 10
                    }
                    radius: 6
                    color: root.palette.window
                }
            }
        }
    }
}
