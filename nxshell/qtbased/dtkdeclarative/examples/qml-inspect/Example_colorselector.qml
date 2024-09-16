// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0

Rectangle {
    Label {
        id: changePaletteLabel
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.topMargin: 20
        text: "Change Palette By object property"
        font.pointSize: 14
        font.bold: true
    }

    Control {
        id: control1
        anchors.left: parent.left
        anchors.top: changePaletteLabel.bottom
        anchors.topMargin: 20
        width: 500
        height: 50

        property Palette backgroundColor: Palette {
            normal: ("black")
        }
        property Palette backgroundColorClicked: Palette {
            normal: ("blue")
        }

        property Palette textColorClicked: Palette {
            normal: ("yellow")
        }

        Rectangle {
            property Palette borderColor: Palette {
                normal: ("red")
            }
            id: rect1
            anchors.left: parent.left
            anchors.top: control1.top
            width: 250
            height: 50
            color: ColorSelector.backgroundColor
            border.width: 1
            border.color: ColorSelector.borderColor

            Text {
                property Palette textColor: Palette {
                    normal: ("white")
                }

                id: rect1Text
                anchors.fill: parent
                text: "Use it's control parent's palette.\n(Click to change.)"
                color: ColorSelector.textColor
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        control1.backgroundColor.normal = "red"
                        rect1Text.textColor.normal = "black"
                    }
                }
            }
        }

        Rectangle {
            id: rect2
            property Palette borderColor: Palette {
                normal: ("red")
            }
            property Palette backgroundColor: Palette {
                normal: ("black")
            }
            property Palette backgroundColorClicked: Palette {
                normal: ("magenta")
            }

            anchors.left: rect1.right
            anchors.leftMargin: 20
            anchors.top: control1.top
            width: 250
            height: 50
            color: ColorSelector.backgroundColor
            border.width: 1
            border.color: ColorSelector.borderColor

            Text {
                id: rect2Text
                property Palette textColor: Palette {
                    normal: ("yellow")
                }

                property Palette textColorClicked: Palette {
                    normal: ("blue")
                }

                anchors.fill: parent
                text: "Overwrite it's control parent palette.\n(Click to change.)"
                color: ColorSelector.textColor
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter

                MouseArea {
                    anchors.fill: parent
                    Palette {
                        id: otherBackgroundColor
                        normal: ("darkRed")
                    }
                    Palette {
                        id: othertextColor
                        normal: ("darkBlue")
                    }

                    onClicked: {
                        rect2.backgroundColor = otherBackgroundColor
                        rect2Text.textColor = othertextColor
                    }
                }
            }
        }
    }

    Label {
        id: changeObjectLabel
        anchors.left: parent.left
        anchors.top: control1.bottom
        anchors.topMargin: 20
        text: "Change Palette By color selector"
        font.pointSize: 14
        font.bold: true
    }

    Control {
        id: control2
        anchors.left: parent.left
        anchors.top: changeObjectLabel.bottom
        anchors.topMargin: 20
        width: 500
        height: 50

        property Palette backgroundColor: Palette {
            normal: ("black")
        }
        property Palette backgroundColorClicked: Palette {
            normal: ("blue")
        }

        property Palette textColorClicked: Palette {
            normal: ("cyan")
        }

        Rectangle {
            property Palette borderColor: Palette {
                normal: ("red")
            }
            id: rect3
            anchors.left: parent.left
            anchors.top: control2.top
            width: 250
            height: 50
            color: ColorSelector.backgroundColor
            border.width: 1
            border.color: ColorSelector.borderColor

            Text {
                property Palette textColor: Palette {
                    normal: ("white")
                }
                id: rect3Text
                anchors.fill: parent
                text: "Use it's color seletor to change palette.\n(Click to change, double click restore)"
                color: ColorSelector.textColor
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        rect3.ColorSelector.backgroundColor = control2.backgroundColorClicked
                        rect3Text.ColorSelector.textColor = control2.textColorClicked
                    }

                    onDoubleClicked: {
                        rect3.ColorSelector.backgroundColor = undefined
                        rect3Text.ColorSelector.textColor = undefined
                    }
                }
            }
        }

        Rectangle {
            id: rect4
            anchors.left: rect3.right
            anchors.top: control2.top
            anchors.leftMargin: 20
            width: 250
            height: 50
            color: ColorSelector.backgroundColor
            property Palette backgroundColor: Palette {
                normal: ("black")
            }
            property Palette backgroundColorClicked: Palette {
                normal: ("magenta")
            }

            Text {
                id: rect4Text
                property Palette textColor: Palette {
                    normal: ("yellow")
                }

                property Palette textColorClicked: Palette {
                    normal: ("blue")
                }

                anchors.fill: parent
                text: "Use CS to replace it's own palette.\n(Click to change, double click restore)"
                color: ColorSelector.textColor
                horizontalAlignment: Qt.AlignHCenter
                verticalAlignment: Qt.AlignVCenter

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        rect4.ColorSelector.backgroundColor = rect4.backgroundColorClicked
                        rect4Text.ColorSelector.textColor = control2  .textColorClicked
                    }

                    onDoubleClicked: {
                        rect4.ColorSelector.backgroundColor = undefined
                        rect4Text.ColorSelector.textColor = undefined
                    }
                }
            }
        }

        Label {
            id: changeParentLabel
            anchors.left: parent.left
            anchors.top: control2.bottom
            anchors.topMargin: 20
            text: "Change palette when parent changed"
            font.pointSize: 14
            font.bold: true
        }

        Rectangle {
            id: changeParentRect
            anchors.left: parent.left
            anchors.top: changeParentLabel.bottom
            anchors.topMargin: 20
            width: 500
            height: 50

            Control {
                id: control3
                width: 220
                height: 50
                anchors.left: parent.left
                anchors.top: parent.top
                property Palette backgroundColor: Palette {
                    normal: ("gray")
                }

                Rectangle {
                    property Palette borderColor: Palette {
                        normal: ("red")
                    }
                    id: reparentRect1
                    width: 50
                    height: 50
                    color: ColorSelector.backgroundColor
                    border.width: 1
                    border.color: ColorSelector.borderColor

                    Text {
                        property Palette textColor: Palette {
                            normal: ("blue")
                        }
                        anchors.fill: parent
                        text: "Rect1"
                        color: ColorSelector.textColor
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (reparentRect1.parent == control3)
                                    reparentRect1.state = "reparent2"
                                else
                                    reparentRect1.state = "reparent1"
                            }
                        }
                    }

                    states: [
                        State {
                            name: "reparent1"
                            ParentChange {
                                target: reparentRect1
                                parent: control3
                                x: 0
                                y: 0
                            }
                        },
                        State {
                            name: "reparent2"
                            ParentChange {
                                target: reparentRect1
                                parent: control4
                                x: 60
                                y: 0
                            }
                        }
                    ]
                }
            }

            Control {
                id: control4
                width: 220
                height: 50
                anchors.left: control3.right
                anchors.leftMargin: 10
                anchors.top: parent.top

                property Palette backgroundColor: Palette {
                    normal: ("red")
                }

                Rectangle {
                    property Palette borderColor: Palette {
                        normal: ("gray")
                    }
                    id: reparentRect2
                    width: 50
                    height: 50
                    color: ColorSelector.backgroundColor
                    border.width: 1
                    border.color: ColorSelector.borderColor

                    Text {
                        property Palette textColor: Palette {
                            normal: ("yellow")
                        }
                        anchors.fill: parent
                        text: "Rect2"
                        color: ColorSelector.textColor
                        horizontalAlignment: Qt.AlignHCenter
                        verticalAlignment: Qt.AlignVCenter

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (reparentRect2.parent == control3)
                                    reparentRect2.state = "reparent2"
                                else
                                    reparentRect2.state = "reparent1"
                            }
                        }
                    }

                    states: [
                        State {
                            name: "reparent1"
                            ParentChange {
                                target: reparentRect2
                                parent: control3
                                x: 60
                                y: 0
                            }
                        },
                        State {
                            name: "reparent2"
                            ParentChange {
                                target: reparentRect2
                                parent: control4
                                x: 0
                                y: 0
                            }
                        }
                    ]
                }
            }
        }

        Label {
            id: customControlLabel
            anchors.left: parent.left
            anchors.top: changeParentRect.bottom
            anchors.topMargin: 20
            text: "Custom control which used the color selector"
            font.pointSize: 14
            font.bold: true
        }

        Row {
            id: dontOverwriteControls
            width: parent.width
            height: 50
            anchors.left: parent.left
            anchors.top: customControlLabel.bottom
            anchors.topMargin: 20
            spacing: 5

            Repeater {
                model: 5
                Example_customcontrolforcs {
                    width: 50
                    height: 50
                    text: modelData
                }
            }
        }

        Row {
            id: overwriteControls
            width: parent.width
            height: 50
            anchors.left: parent.left
            anchors.top: dontOverwriteControls.bottom
            anchors.topMargin: 20
            spacing: 5

            Repeater {
                model: 5
                Example_customcontrolforcs {
                    backgroundColor: Palette {
                        normal: Qt.hsla(Math.random(), 0.6, 0.6, 0.9)
                        hovered: Qt.hsla(Math.random(), 0.2, 0.2, 0.9)
                    }

                    width: 50
                    height: 50
                    text: modelData
                }
            }
        }
    }
}
