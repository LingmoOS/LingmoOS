// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.11
import org.deepin.dtk 1.0

Rectangle {
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: function(mouse) {
            if (mouse.button === Qt.RightButton)
                contextMenu.popup()
        }
        onPressAndHold: function(mouse) {
            if (mouse.source === Qt.MouseEventNotSynthesized)
                contextMenu.popup()
        }

        Menu {
            id: contextMenu
            MenuItem {
                text: "Cut"
                icon.name: "edit-cut"
                display: AbstractButton.TextBesideIcon
            }
            MenuItem {
                text: "Copy"
                icon.name: "edit-copy"
                display: IconLabel.IconBesideText
            }
            MenuItem {
                text: "Paste"
                checkable: true
                checked: true
            }
            MenuSeparator { }
            Menu {
                title: "Find/Replace"
                Action { text: "Find Next" }
                Action { text: "Find Previous" }
                Action { text: "Replace" }
            }
            MenuItem { text: "Exit" }
        }
    }

    Label {
        id: btnLabel
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        height: btn.height
        text: qsTr("Button:")
        verticalAlignment: Text.AlignVCenter
    }

    Button {
        id: btn
        anchors.left: btnLabel.right
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        text: "Button"
    }

    WarningButton {
        id: warningBtn
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20 // @disable-check M16
        anchors.left: btn.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        text: "Warning" // @disable-check M16
        onPressed: console.log("WarningButton pressed") // @disable-check M16
    }

    RecommandButton {
        id: highlightedionBtn
        anchors.left: warningBtn.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20 // @disable-check M16
        text: "Recommand" // @disable-check M16
    }

    ToolButton {
        id: toolButton
        anchors.left: highlightedionBtn.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20 // @disable-check M16
    }

    IconButton {
        id: iconButton
        width: 36  // @disable-check M16
        anchors.left: toolButton.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20 // @disable-check M16
        icon.name: "org.deepin.Example/test/action_add"
    }

    IconButton {
        id: iconButton1
        width: 36  // @disable-check M16
        anchors.left: iconButton.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20 // @disable-check M16
        icon.name: "test/action_add"  // @disable-check M16
    }

    IconButton {
        id: iconButton2
        width: 36  // @disable-check M16
        anchors.left: iconButton1.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20 // @disable-check M16
        flat: true // @disable-check M16
        icon.name: "action_add"  // @disable-check M16
    }

    FloatingButton {
        id: floatingButton
        width: 36  // @disable-check M16
        anchors.left: iconButton2.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20 // @disable-check M16
        icon.name: "action_add"
    }

    DelayButton {
        id: dlyBtn
        anchors.left: floatingButton.right
        anchors.leftMargin: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        text: "DelayButton"

        NumberAnimation on progress {
            from: 0
            to: 1.0
            duration: 4000
            loops: Animation.Infinite
        }
    }

    RoundButton {
        id: roundButton;
        anchors.top: parent.top
        anchors.left: dlyBtn.right
        anchors.leftMargin: 20
        anchors.topMargin: 20

        icon.name: "action_add"
    }

    ButtonBox {
        id: buttonBox
        anchors.left: roundButton.right // @disable-check M16
        anchors.leftMargin: 20 // @disable-check M16
        anchors.top: parent.top // @disable-check M16
        anchors.topMargin: 20  // @disable-check M16

        ToolButton {
            checkable: true; icon.name: "go-previous"; checked: true // @disable-check M16
        }
        ToolButton {
            checkable: true; icon.name: "go-down" // @disable-check M16
        }
        ToolButton {
            checkable: true; icon.name: "go-next" // @disable-check M16
        }
    }

    LineEdit {
        id: lineEdit
        anchors.left: parent.left
        anchors.top: btnLabel.bottom
        anchors.topMargin: 20
        text: "Testing the alert message in line edit."

        alertText: qsTr("This is a long sentence.")
        alertDuration: 2000  // millisecond
        showAlert: focus
    }

    SearchEdit {
        id: searcherEdit
        anchors.left: lineEdit.right
        anchors.top: lineEdit.top
        anchors.leftMargin: 20
    }

    SpinBox {
        id: spinBox
        anchors.left: searcherEdit.right
        anchors.top: searcherEdit.top
        anchors.leftMargin: 20
        editable: true
        alertText: qsTr("This is a long sentence.")
        alertDuration: 2000  // millisecond
        showAlert: focus
    }

    SpinBox {
        id: customSpinBox
        anchors.left: spinBox.right
        anchors.top: spinBox.bottom
        anchors.leftMargin: 20
        editable: true
        value: 1

        up.indicator: Rectangle {
            border.color: "green"
            anchors.right: customSpinBox.right
            anchors.rightMargin: 5
            width: 20
            height: 15
        }
    }

    PlusMinusSpinBox {
        id: plusMinusSpinBox
        anchors.left: spinBox.right
        anchors.top: spinBox.top
        anchors.leftMargin: 20
        spinBox.editable: true
        spinBox.alertText: qsTr("Only numbers can be entered.")
        spinBox.alertDuration: 2000  // millisecond
        spinBox.showAlert: focus
    }

    ListView {
        model: 5
        implicitHeight: 250

        delegate: CheckDelegate {
            text: index;

            icon.name: "action_add"
        }

        anchors.top: btnLabel.bottom
        anchors.left: plusMinusSpinBox.right
        anchors.topMargin: 20
        anchors.leftMargin: 20
    }

    Row {
        id: sliderRow
        anchors {
            top: lineEdit.bottom
            topMargin: 20
        }

        spacing: 10
        height: 350

        Slider {
            highlightedPassedGroove: true
            orientation: Qt.Vertical
            height: parent.height
            handleType: Slider.HandleType.NoArrowVertical
        }

        Slider {
            height: parent.height
            orientation: Qt.Vertical
            handleType: Slider.HandleType.ArrowLeft
        }

        Slider {
            height: parent.height
            highlightedPassedGroove: true
            orientation: Qt.Vertical
            handleType: Slider.HandleType.ArrowRight
        }
    }

    Column {
        id: sliderColum
        anchors {
            top: lineEdit.bottom
            topMargin: 40
            left: sliderRow.right
            leftMargin: 60
        }
        spacing: 10
        width: 400

        Slider {
            highlightedPassedGroove: true
            width: parent.width
        }

        Slider {
            width: parent.width
            handleType: Slider.HandleType.ArrowBottom
        }

        Slider {
            width: parent.width
            highlightedPassedGroove: true
            handleType: Slider.HandleType.ArrowUp
        }

        TipsSlider {
            width: parent.width
            tickDirection: TipsSlider.TickDirection.Back
            slider.highlightedPassedGroove: true
            slider.handleType: Slider.HandleType.ArrowBottom

            ticks: [SliderTipItem {
                },
                SliderTipItem {
                },
                SliderTipItem {
                },
                SliderTipItem {
                }]
        }

        TipsSlider {
            id: sliderTickTip2
            readonly property var tips: [qsTr("Fast"), qsTr("Slow")]
            width: parent.width
            tickDirection: TipsSlider.TickDirection.Back
            slider.handleType: Slider.HandleType.ArrowBottom

            ticks: [SliderTipItem {
                    text: sliderTickTip2.tips[0]
                },
                SliderTipItem {
                    text: sliderTickTip2.tips[1]
                }]

            // test highlight Text
            HighlightPanel {
                id: __highlight
                readonly property real highlightMargin: 20
                anchors.bottom: parent.bottom
                x: parent.slider.value * (parent.width - parent.slider.handle.width) - width / 2
                   + parent.slider.handle.width / 2
                width: __text.implicitWidth
                height: __text.implicitHeight
                Text {
                    id: __text
                    text: sliderTickTip2.tips[0]
                    color: palette.highlightedText
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        TipsSlider {
            id: sliderTickTip3
            readonly property var tips: [qsTr("1m"), qsTr("5m"), qsTr("10m"), qsTr("15m"), qsTr("30m"), qsTr("1h"), qsTr("Never")]
            width: parent.width
            tickDirection: TipsSlider.TickDirection.Back
            slider.handleType: Slider.HandleType.ArrowBottom
            slider.stepSize: 10
            slider.from: 0
            slider.to: 60

            ticks: [SliderTipItem {
                    text: sliderTickTip3.tips[0]
                    textHorizontalAlignment: Text.AlignLeft
                },
                SliderTipItem {
                    text: sliderTickTip3.tips[1]
                },
                SliderTipItem {
                    text: sliderTickTip3.tips[2]
                },
                SliderTipItem {
                    text: sliderTickTip3.tips[3]
                },
                SliderTipItem {
                    text: sliderTickTip3.tips[4]
                },
                SliderTipItem {
                    text: sliderTickTip3.tips[5]
                },
                SliderTipItem {
                    text: sliderTickTip3.tips[6]
                    textHorizontalAlignment: Text.AlignRight
                }]

            // test end Indicator
            Rectangle {
                width: 30
                height: 30
                color: "blue"
                anchors {
                    left: parent.right
                    leftMargin: 20
                    verticalCenter: parent.slider.verticalCenter
                }

                Text {
                    anchors.centerIn: parent
                    text: sliderTickTip3.slider.value.toFixed(1)
                    color: "white"
                }
            }

            // test start Indicator
            Rectangle {
                width: 30
                height: 30
                color: "red"
                anchors {
                    right: parent.left
                    rightMargin: 20
                    verticalCenter: parent.slider.verticalCenter
                }

                Text {
                    anchors.centerIn: parent
                    text: "S"
                    color: "white"
                }
            }
        }
    }

    Row {
        id: switchBtnRow
        anchors {
            top: sliderColum.bottom
            left: sliderRow.right
            topMargin: 30
            leftMargin: 40
        }
        spacing: 10
        Switch {
            checked: true
        }

        Switch {
        }

        Switch {
            checked: true
            enabled: false
        }

        Switch {
            enabled: false
        }
    }

    Row {
        anchors {
            top: sliderColum.bottom
            left: switchBtnRow.right
            topMargin: 30
            leftMargin: 20
        }

        spacing: 10

        ComboBox {
            id: nonEditComboBox
            model: ["Banana", "Apple", "Coconut"]
        }

        ComboBox {
            id: editableComboBox
            editable: true
            model: ["Banana", "Apple", "Coconut"]
        }

        ComboBox {
            id: nonEditComboBoxWithIcon
            textRole: "text"
            iconNameRole: "icon"

            model: ListModel {
                ListElement { text: "Banana"; icon: "go-previous" }
                ListElement { text: "Apple"; icon: "go-down" }
                ListElement { text: "Coconut"; icon: "go-next" }
            }
        }

        ComboBox {
            id: editableComboBoxWithIcon
            editable: true
            textRole: "text"
            iconNameRole: "icon"

            model: ListModel {
                ListElement { text: "Banana"; icon: "go-previous" }
                ListElement { text: "Apple"; icon: "go-down" }
                ListElement { text: "Coconut"; icon: "go-next" }
            }
        }
    }
}
