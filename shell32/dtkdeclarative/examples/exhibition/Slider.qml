// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import QtQuick.Layouts 1.11
import org.deepin.dtk 1.0

Column {
    id: control
    Label {
        width: parent.width
        wrapMode: Text.WordWrap
        text: "圆角矩形滑块可以随意拖动，起止点一定是从左往右递增，滑块以左部分是活动色显示。尖角的滑块不可以像圆角矩形滑块那样进行随意拖动，底下对应的有刻度，刻度上产生吸附力，尖角也只能在几个刻度值上调整，更多强调的是用户一个值的取舍。 对应的刻度可能有刻度值显示，也可能没有刻度值（界面上的）。"
        horizontalAlignment: Qt.AlignHCenter
    }

    spacing: 10

    property int leftAreaWidth: 300
    property int rightAreaWidth: 600
    function toPercent(value) {
        return Number(value * 100).toFixed(0) + "%"
    }

    Flow {
        spacing: 30
        width: parent.width

        Slider {
            highlightedPassedGroove: true
            width: Math.max(implicitWidth, leftAreaWidth)
            handleType: Slider.HandleType.NoArrowHorizontal
            value: 0.5
        }

        RowLayout {
            width: Math.max(implicitWidth, rightAreaWidth)
            spacing: 30
            Text { text: "视图大小" }
            Slider {
                Layout.fillWidth: true
                highlightedPassedGroove: true
                handleType: Slider.HandleType.NoArrowHorizontal
                value: 0.5
            }
        }

        Item { width: parent.width; height: 1}

        RowLayout {
            width: Math.max(implicitWidth, leftAreaWidth)
            spacing: 10
            DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
            Slider {
                Layout.fillWidth: true
                highlightedPassedGroove: true
                handleType: Slider.HandleType.NoArrowHorizontal
                value: 0.5
            }
            DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
        }

        ColumnLayout {
            width: Math.max(implicitWidth, rightAreaWidth)
            spacing: 30

            RowLayout {
                Layout.preferredWidth: parent.width
                Text { text: "输出音量" }
                Label {
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    text: toPercent(voiceTipsSlider.value)
                }
            }
            RowLayout {
                spacing: 10
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
                Slider {
                    id: voiceTipsSlider
                    Layout.fillWidth: true
                    highlightedPassedGroove: true
                    handleType: Slider.HandleType.NoArrowHorizontal
                    value: 0.5
                }
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
            }
        }

        Item { width: parent.width; height: 1}

        ColumnLayout {
            width: Math.max(implicitWidth, leftAreaWidth)
            height: arrwoSliders.height
            Slider {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                Layout.topMargin: 46
                handleType: Slider.HandleType.ArrowBottom
                value: 0.5
            }

            RowLayout {
                spacing: 10
                Layout.alignment: Qt.AlignBottom
                Layout.bottomMargin: 10
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
                Slider {
                    Layout.fillWidth: true
                    handleType: Slider.HandleType.ArrowBottom
                    value: 0.5
                }
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
            }
        }

        ColumnLayout {
            id: arrwoSliders
            width: Math.max(implicitWidth, rightAreaWidth)
            spacing: 20
            ColumnLayout {
                Layout.preferredWidth: parent.width
                spacing: 30

                Text { text: "进入睡眠" }
                TipsSlider {
                    id: sleepSlider
                    Layout.fillWidth: true
                    tickDirection: TipsSlider.TickDirection.Back
                    slider.handleType: Slider.HandleType.ArrowBottom
                    slider.value: 3
                    slider.from: 0
                    slider.to: ticks.length - 1
                    slider.stepSize: 1
                    ticks: Repeater {
                        model: ["1m", "5m", "10m", "15m", "30m", "1h", "从不"]
                        delegate: SliderTipItem {
                            text: modelData
                            highlight: sleepSlider.slider.value === index
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.preferredWidth: parent.width
                spacing: 30

                Text { text: "双击速度" }
                TipsSlider {
                    Layout.fillWidth: true
                    tickDirection: TipsSlider.TickDirection.Back
                    slider.handleType: Slider.HandleType.ArrowBottom
                    slider.value: 0.5
                    ticks: Repeater {
                        model: ["慢", "", "", "", "", "", "快"]
                        delegate: SliderTipItem {
                            text: modelData
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.preferredWidth: parent.width
                spacing: 30

                Text { text: "音量平衡" }
                RowLayout {
                    Layout.preferredWidth: parent.width
                    spacing: 10
                    DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
                    TipsSlider {
                        Layout.fillWidth: true
                        tickDirection: TipsSlider.TickDirection.Back
                        slider.handleType: Slider.HandleType.ArrowBottom
                        slider.value: 0.5
                        ticks: Repeater {
                            model: ["", "", "", "", "", "", ""]
                            delegate: SliderTipItem {
                                text: modelData
                            }
                        }
                    }
                    DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16)}
                }
            }
        }

        Item { width: parent.width; height: 1}

        RowLayout {
            width: Math.max(implicitWidth, rightAreaWidth)

            Slider {
                highlightedPassedGroove: true
                Layout.preferredHeight: 300
                handleType: Slider.HandleType.NoArrowVertical
                value: 0.5
                orientation: Qt.Vertical
            }

            ColumnLayout {
                height: 150
                spacing: 10
                Layout.alignment: Qt.AlignHCenter
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16); Layout.alignment: Qt.AlignHCenter}
                Slider {
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter
                    highlightedPassedGroove: true
                    handleType: Slider.HandleType.NoArrowVertical
                    value: 0.5
                    orientation: Qt.Vertical
                }
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16); Layout.alignment: Qt.AlignHCenter}
            }
            ColumnLayout {
                height: 150
                spacing: 10
                Layout.alignment: Qt.AlignHCenter
                Label { text:toPercent(voiceVerticalSlider.value); Layout.alignment: Qt.AlignHCenter}
                Slider {
                    id: voiceVerticalSlider
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignHCenter
                    highlightedPassedGroove: true
                    handleType: Slider.HandleType.NoArrowVertical
                    value: 0.5
                    orientation: Qt.Vertical
                }
                DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16); Layout.alignment: Qt.AlignHCenter}
            }

            //StyledArrowShapeBlur {
            //    width: 60
            //    height: 250
            //    control: voiceVerticalSlider2
            //    ColumnLayout {
            //        height: 200
            //        spacing: 10
            //        anchors {
            //            horizontalCenter: parent.horizontalCenter
            //            verticalCenter: parent.verticalCenter
            //        }
            //        Label { text:toPercent(voiceVerticalSlider2.value); Layout.alignment: Qt.AlignHCenter}
            //        Slider {
            //            id: voiceVerticalSlider2
            //            Layout.fillHeight: true
            //            Layout.alignment: Qt.AlignHCenter
            //            highlightedPassedGroove: true
            //            handleType: Slider.HandleType.NoArrowVertical
            //            value: 0.5
            //            orientation: Qt.Vertical
            //        }
            //        DciIcon { name: "entry_voice"; sourceSize: Qt.size(16, 16); Layout.alignment: Qt.AlignHCenter}
            //    }
            //}
        }
    }
}
