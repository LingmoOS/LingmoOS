// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15
import QtQuick.Controls
import QtQuick.Layouts
import org.deepin.ds

Window {
    id: root
    height: 150
    width: 780
    visible: true
    DLayerShellWindow.anchors: DLayerShellWindow.AnchorLeft
    DLayerShellWindow.layer: DLayerShellWindow.LayerTop
    DLayerShellWindow.exclusionZone: excludezone.value
    DLayerShellWindow.keyboardInteractivity: DLayerShellWindow.KeyboardInteractivityExclusive
    DLayerShellWindow.leftMargin: left.value
    DLayerShellWindow.rightMargin: right.value
    DLayerShellWindow.topMargin: top.value
    DLayerShellWindow.bottomMargin: bottom.value
    DLayerShellWindow.closeOnDismissed: false

    // default bind to primary screen
    screen: Application.screens[0]

    Column {
        id: comboBox
        ComboBox {
            id: layerCombox
            width: 100
            textRole: "text"
            anchors.left: parent.left
            Component.onCompleted: currentIndex = root.DLayerShellWindow.layer
            onActivated: {
                root.DLayerShellWindow.layer = model.get(layerCombox.currentIndex).layer
            }
            model: ListModel {
                ListElement { text: "LayerBackground"; layer: DLayerShellWindow.LayerBackground }
                ListElement { text: "LayerButtom"; layer: DLayerShellWindow.LayerButtom }
                ListElement { text: "LayerTop"; layer: DLayerShellWindow.LayerTop }
                ListElement { text: "LayerOverlay"; layer: DLayerShellWindow.LayerOverlay }
            }
        }

        ComboBox {
            id: keyboardInteractivityCombox
            width: 200
            textRole: "text"
            anchors.left: parent.left
            Component.onCompleted: currentIndex = root.DLayerShellWindow.keyboardInteractivity
            onActivated: {
                root.DLayerShellWindow.keyboardInteractivity = model.get(keyboardInteractivityCombox.currentIndex).keyboardInteractivity
            }
            model: ListModel {
                ListElement { text: "KeyboardInteractivityNone"; keyboardInteractivity: DLayerShellWindow.KeyboardInteractivityNone }
                ListElement { text: "KeyboardInteractivityExclusive"; keyboardInteractivity: DLayerShellWindow.KeyboardInteractivityExclusive }
                ListElement { text: "KeyboardInteractivityOnDemand"; keyboardInteractivity: DLayerShellWindow.KeyboardInteractivityOnDemand }
            }
        }

        ComboBox {
            id: screenComboBox
            width: 200
            textRole: "name"
            anchors.left: parent.left
            onActivated: {
                root.screen = model[screenComboBox.currentIndex]
            }
            model: Application.screens
        }
    }

    Item {
        id: steerwheel
        anchors.left: comboBox.right
        anchors.leftMargin: 20
        width: 100
        Column {
            spacing: 20
            Button {
                id: buttonTop
                text: "Top"
                width: 40
                anchors.leftMargin: 30
                anchors.left: parent.left
                checkable: true
                checked: root.DLayerShellWindow.anchors & DLayerShellWindow.AnchorTop
                onCheckedChanged: {
                    if (checked)
                        root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors | DLayerShellWindow.AnchorTop
                    else
                        root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors ^ DLayerShellWindow.AnchorTop
                }
            }
            Row {
                spacing: 20
                Button {
                    id: buttonLeft
                    text: "Left"
                    width: 40
                    checkable: true
                    checked: root.DLayerShellWindow.anchors & DLayerShellWindow.AnchorLeft
                    onCheckedChanged: {
                        if (checked)
                            root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors | DLayerShellWindow.AnchorLeft
                        else
                            root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors ^ DLayerShellWindow.AnchorLeft
                    }
                }
                Button {
                    id: buttonRight
                    text: "Right"
                    width: 40
                    checkable: true
                    checked: root.DLayerShellWindow.anchors & DLayerShellWindow.AnchorRight
                    onCheckedChanged: {
                        if (checked)
                            root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors | DLayerShellWindow.AnchorRight
                        else
                            root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors ^ DLayerShellWindow.AnchorRight
                    }
                }
            }
            Button {
                id: buttonBottom
                text: "Buttom"
                width: 40
                anchors.leftMargin: 30
                anchors.left: parent.left
                checkable: true
                checked: root.DLayerShellWindow.anchors & DLayerShellWindow.AnchorBottom
                onCheckedChanged: {
                    if (checked)
                        root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors | DLayerShellWindow.AnchorBottom
                    else
                        root.DLayerShellWindow.anchors = root.DLayerShellWindow.anchors ^ DLayerShellWindow.AnchorBottom
                }
            }
        }
    }

    GridLayout {
        id: input
        anchors.left: steerwheel.right
        anchors.leftMargin: 20
        columns: 4
        Text {
            text: "LeftMargin: "
        }
        SpinBox {
            id: left
            stepSize: 10
            editable: true
            value: root.DLayerShellWindow.leftMargin
        }
        Text {
            text: "TopMargin: "
        }
        SpinBox {
            id: top
            stepSize: 10
            editable: true
            value: root.DLayerShellWindow.topMargin
        }
        Text {
            text: "RightMargin: "
        }
        SpinBox {
            id: right
            stepSize: 10
            editable: true
            value: root.DLayerShellWindow.rightMargin
        }
        Text {
            text: "BottomMargin: "
        }
        SpinBox {
            id: bottom
            stepSize: 10
            editable: true
            value: root.DLayerShellWindow.bottomMargin
        }
        Text {
            text: "excludezone: "
        }

        SpinBox {
            id: excludezone
            editable: true
            from: -1
            to: 1080
            value: -1
        }
    }
}
