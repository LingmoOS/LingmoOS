// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick 2.0
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import org.lingmo.dtk 1.0 as D
import org.lingmo.dtk.style 1.0 as DS
import org.lingmo.oceanui 1.0

OceanUIObject {
    id: touchpad
    property bool enabled : oceanuiData.tapEnabled
    OceanUIObject {
        name: "BasicSettings"
        parentName: "MouseAndTouchpad/Touchpad"
        displayName: qsTr("Basic Settings")
        weight: 10
        pageType: OceanUIObject.Item

        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }
        }
    }

    OceanUIObject {
        name: "DisableTouchpadByMouse"
        parentName: "MouseAndTouchpad/Touchpad"
        displayName: qsTr("Touchpad")
        weight: 20
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Editor
        page: D.Switch {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            checked: oceanuiData.tapEnabled
            onCheckedChanged: {
                if (checked !== oceanuiData.tapEnabled) {
                    oceanuiData.tapEnabled = checked;
                }
            }
        }
    }

    OceanUIObject {
        name: "PointerSpeed"
        parentName: "MouseAndTouchpad/Touchpad"
        displayName: qsTr("Pointer Speed")
        weight: 30
        visible: touchpad.enabled
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item

        page: ColumnLayout {
            Label {
                id: speedText

                Layout.topMargin: 10
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t7
                text: oceanuiObj.displayName
            }

            D.TipsSlider {
                id: scrollSlider

                readonly property var tips: [qsTr("Slow"), (""), (""), (""), (""), (""), qsTr("Fast")]

                Layout.preferredHeight: 90
                Layout.alignment: Qt.AlignCenter
                Layout.margins: 10
                Layout.fillWidth: true
                tickDirection: D.TipsSlider.TickDirection.Back
                slider.handleType: Slider.HandleType.ArrowBottom
                slider.value: oceanuiData.tpadMoveSpeed
                slider.from: 0
                slider.to: ticks.length - 1
                slider.live: true
                slider.stepSize: 1
                slider.snapMode: Slider.SnapAlways
                slider.onValueChanged: {
                    oceanuiData.tpadMoveSpeed = slider.value;
                }
                ticks: [
                    D.SliderTipItem {
                        text: scrollSlider.tips[0]
                        highlight: scrollSlider.slider.value === 0
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[1]
                        highlight: scrollSlider.slider.value === 1
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[2]
                        highlight: scrollSlider.slider.value === 2
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[3]
                        highlight: scrollSlider.slider.value === 3
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[4]
                        highlight: scrollSlider.slider.value === 4
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[5]
                        highlight: scrollSlider.slider.value === 5
                    },
                    D.SliderTipItem {
                        text: scrollSlider.tips[6]
                        highlight: scrollSlider.slider.value === 6
                    }
                ]
            }

        }

    }

    OceanUIObject {
        name: "TouchpadGroup"
        parentName: "MouseAndTouchpad/Touchpad"
        displayName: qsTr("Pointer Speed")
        weight: 40
        visible: touchpad.enabled
        pageType: OceanUIObject.Item

        OceanUIObject {
            name: "DisableTouchpadByInput"
            parentName: "MouseAndTouchpad/Touchpad/TouchpadGroup"
            displayName: qsTr("Disable touchpad during input")
            weight: 10
            pageType: OceanUIObject.Editor

            page: D.Switch {
                Layout.rightMargin: 10
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                checked: oceanuiData.disIfTyping
                onCheckedChanged: {
                    if (checked !== oceanuiData.disIfTyping) {
                        oceanuiData.disIfTyping = checked;
                    }
                }
            }

        }

        OceanUIObject {
            name: "TapToClick"
            parentName: "MouseAndTouchpad/Touchpad/TouchpadGroup"
            displayName: qsTr("Tap to Click")
            weight: 20
            pageType: OceanUIObject.Editor

            page: D.Switch {
                Layout.rightMargin: 10
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                checked: oceanuiData.tapClick
                onCheckedChanged: {
                    if (checked !== oceanuiData.tapClick) {
                        oceanuiData.tapClick = checked;
                    }
                }
            }

        }

        OceanUIObject {
            name: "NaturalScrolling"
            parentName: "MouseAndTouchpad/Touchpad/TouchpadGroup"
            displayName: qsTr("Natural Scrolling")
            weight: 30
            pageType: OceanUIObject.Editor

            page: D.Switch {
                Layout.rightMargin: 10
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                checked: oceanuiData.tpadNaturalScroll
                onCheckedChanged: {
                    if (checked !== oceanuiData.tpadNaturalScroll) {
                        oceanuiData.tpadNaturalScroll = checked;
                    }
                }
            }

        }

        page: OceanUIGroupView {
        }

    }

    OceanUIObject {
        name: "Gesture"
        parentName: "MouseAndTouchpad/Touchpad"
        displayName: qsTr("Gesture")
        weight: 50
        pageType: OceanUIObject.Item
        visible: false

        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }

        }

    }

    OceanUIObject {
        name: "GestureGroup"
        parentName: "MouseAndTouchpad/Touchpad"
        weight: 60
        pageType: OceanUIObject.Item
        visible: false

        OceanUIObject {
            name: "animation"
            parentName: "MouseAndTouchpad/Touchpad/GestureGroup"
            weight: 10
            pageType: OceanUIObject.Item

            page: Rectangle {
                color: "transparent"
                implicitHeight: rowView.height + 20

                RowLayout {
                    id: rowView

                    width: parent.width
                    anchors.centerIn: parent

                    Image {
                        source: "qrc:/icons/lingmo/builtin/icons/oceanui_nav_touchpad_1_42px.png"
                        // fillMode: Image.PreserveAspectFit
                        Layout.alignment: Qt.AlignCenter
                        sourceSize.width: 204
                        sourceSize.height: 134
                    }

                    Image {
                        source: "qrc:/icons/lingmo/builtin/icons/oceanui_nav_touchpad_2_42px.png"
                        Layout.alignment: Qt.AlignCenter
                        sourceSize.width: 204
                        sourceSize.height: 134
                    }

                }

            }

        }

        page: OceanUIGroupView {
        }

    }

    OceanUIObject {
        name: "ThreeFingerGesture"
        parentName: "MouseAndTouchpad/Touchpad"
        displayName: qsTr("Three-finger gestures")
        weight: 70
        backgroundType: OceanUIObject.AutoBg
        pageType: OceanUIObject.Item
        visible: false

        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }

        }

    }

    OceanUIObject {
        name: "ThreeFingerGestureGroup"
        parentName: "MouseAndTouchpad/Touchpad"
        weight: 80
        backgroundType: OceanUIObject.AutoBg
        //hasBackground: false
        visible: false
        pageType: OceanUIObject.Item

        OceanUIObject {
            name: "ThreeFingersUp"
            parentName: "MouseAndTouchpad/Touchpad/ThreeFingerGestureGroup"
            displayName: qsTr("Three-finger up")
            icon: "Two_fingers_up_and_down"
            weight: 20
            pageType: OceanUIObject.Editor

            page: D.ComboBox {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                model: [qsTr("Scroll Page")]
                currentIndex: 0
            }

        }

        OceanUIObject {
            name: "ThreeFingersDown"
            parentName: "MouseAndTouchpad/Touchpad/ThreeFingerGestureGroup"
            displayName: qsTr("Three-finger down")
            icon: "Two_fingers_up_and_down"
            weight: 30
            pageType: OceanUIObject.Editor

            page: D.ComboBox {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                model: [qsTr("Scroll Page")]
                currentIndex: 0
            }

        }

        OceanUIObject {
            name: "ThreeFingersLeft"
            parentName: "MouseAndTouchpad/Touchpad/ThreeFingerGestureGroup"
            displayName: qsTr("Three-finger left")
            icon: "Two_fingers_up_and_down"
            weight: 40
            pageType: OceanUIObject.Editor

            page: D.ComboBox {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                model: [qsTr("Scroll Page")]
                currentIndex: 0
            }

        }

        OceanUIObject {
            name: "ThreeFingersRight"
            parentName: "MouseAndTouchpad/Touchpad/ThreeFingerGestureGroup"
            displayName: qsTr("Three-finger right")
            icon: "Two_fingers_up_and_down"
            weight: 50
            pageType: OceanUIObject.Editor

            page: D.ComboBox {
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                model: [qsTr("Scroll Page")]
                currentIndex: 0
            }

        }

        page: OceanUIGroupView {
        }

    }

    OceanUIObject {
        name: "FourFingerGesture"
        parentName: "MouseAndTouchpad/Touchpad"
        displayName: qsTr("Four-finger gestures")
        weight: 90
        backgroundType: OceanUIObject.AutoBg
        pageType: OceanUIObject.Item
        visible: false
        page: ColumnLayout {
            Label {
                Layout.leftMargin: 10
                font: D.DTK.fontManager.t4
                text: oceanuiObj.displayName
            }

        }

    }

    OceanUIObject {
        name: "FourFingerGestureGroup"
        parentName: "MouseAndTouchpad/Touchpad"
        weight: 100
        backgroundType: OceanUIObject.Normal
        pageType: OceanUIObject.Item
        visible: false
        page: GestureGroup {

            model: ListModel {
                ListElement {
                    name: qsTr("Four-finger up")
                    iconName: "Two_fingers_up_and_down"
                }

                ListElement {
                    name: qsTr("Four-finger down")
                    iconName: "Two_fingers_up_and_down"
                }

                ListElement {
                    name: qsTr("Four-finger left")
                    iconName: "Two_fingers_up_and_down"
                }

                ListElement {
                    name: qsTr("Four-finger right")
                    iconName: "Two_fingers_up_and_down"
                }

                ListElement {
                    name: qsTr("Four-finger tap")
                    iconName: "Two_fingers_up_and_down"
                }

            }

        }

    }

}
