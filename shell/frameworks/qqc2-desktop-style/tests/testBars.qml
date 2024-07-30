/*
 *  SPDX-FileCopyrightText: 2023 ivan (@ratijas) tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    title: "Progress Bars & Sliders"
    width: demo.implicitWidth
    height: demo.implicitHeight

    component LoopAnimation : SequentialAnimation {
        id: animation

        property real to: 1

        running: true

        // this is bound to global time, so that you could open multiple
        // windows side by side, and they all play in sync
        ScriptAction {
            script: phase.value = value() * animation.to
            function value() {
                var phase = Date.now() % 7000;
                if (phase < 500) {
                    return 0;
                } else if (phase < 3500) {
                    return (phase - 500) / 3000;
                } else if (phase < 4000) {
                    return 1;
                } else {
                    return 1 - ((phase - 4000) / 3000);
                }
            }
        }
        PropertyAction {
            id: phase
        }
        PauseAnimation { duration: 1 }
        onFinished: restart()
    }

    component Demo : GridLayout {
        id: layout
        columns: 4
        columnSpacing: Kirigami.Units.smallSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        Slider {
            orientation: Qt.Vertical
            to: 10
            value: 3
            stepSize: 1
            LoopAnimation on value {
                to: 10
            }
            Layout.fillHeight: true
            Layout.rowSpan: 11
        }

        Slider {
            orientation: Qt.Vertical
            to: 10
            value: 3
            stepSize: 1
            Layout.fillHeight: true
            Layout.rowSpan: 11
        }

        Label {
            text: "Mirrorring:"
            Layout.alignment: Qt.AlignRight
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Label {
                text: layout.LayoutMirroring.enabled ? "Enabled" : "Disabled"
                Layout.rightMargin: Kirigami.Units.gridUnit * 2
            }

            Label {
                text: "Inherit:"
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: layout.LayoutMirroring.childrenInherit ? "Yes" : "No"
            }
        }

        Label {
            text: "Value:"
            Layout.alignment: Qt.AlignRight
        }

        Label {
            text: value.toFixed(2)
            property real value
            LoopAnimation on value {}
            Layout.fillWidth: true
        }

        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit
            Layout.columnSpan: 2
        }

        Label {
            text: "Progress Bar:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        ProgressBar {
            LoopAnimation on value {}
            Layout.fillWidth: true
        }

        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit
            Layout.columnSpan: 2
        }

        Label {
            text: "Indeterminate:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        ProgressBar {
            indeterminate: true
            Layout.fillWidth: true
        }

        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit
            Layout.columnSpan: 2
        }

        Label {
            text: "Slider:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        Slider {
            stepSize: 0.1
            LoopAnimation on value {}
            Layout.fillWidth: true
        }

        Item {
            Layout.preferredHeight: Kirigami.Units.gridUnit
            Layout.columnSpan: 2
        }

        Label {
            text: "Manual:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        // Test for BUG-455339
        Slider {
            to:       300000
            stepSize: 30000
            value:    90000
            Layout.fillWidth: true
        }

        Label {
            text: "Non-live:"
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing
            Layout.fillWidth: true

            // Test for BUG-473400
            Slider {
                id: nonLiveSlider
                live: false
                to:       300000
                stepSize: 30000
                value:    90000
                Layout.fillWidth: true
            }

            Label {
                text: String(Math.round(nonLiveSlider.value))
                horizontalAlignment: Text.AlignRight
                Layout.preferredWidth: nonLiveSliderTextMetrics.advanceWidth
                LayoutMirroring.enabled: false
            }

            TextMetrics {
                id: nonLiveSliderTextMetrics
                font: nonLiveSlider.font
                text: String(nonLiveSlider.to)
            }
        }
    }

    Kirigami.Page {
        id: demo

        anchors.fill: parent
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            RowLayout {
                Layout.fillWidth: false
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignHCenter
                spacing: Kirigami.Units.smallSpacing

                Label {
                    text: "Application Layout Direction:"
                    Layout.alignment: Qt.AlignRight
                }
                Label {
                    text: Qt.application.layoutDirection === Qt.RightToLeft ? "Right to Left" : "Left to Right"
                }
            }
            Kirigami.Separator { Layout.fillWidth: true }
            RowLayout {
                Layout.fillHeight: false
                spacing: Kirigami.Units.largeSpacing

                Demo {
                    id: sampleDemo
                    LayoutMirroring.enabled: true
                    LayoutMirroring.childrenInherit: true
                }
                Kirigami.Separator { Layout.fillHeight: true }
                Demo {
                    LayoutMirroring.enabled: false
                    LayoutMirroring.childrenInherit: true
                }
                Layout.bottomMargin: Kirigami.Units.gridUnit
            }
            Kirigami.Separator { Layout.fillWidth: true }
            Demo {
                Layout.fillWidth: false
                Layout.fillHeight: false
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: sampleDemo.width
            }
            Item {
                Layout.fillHeight: true
            }
            Kirigami.SelectableLabel {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.MarkdownText
                text: "Hint: run this test both with `export LANGUAGE=ar:en_US LANG=ar_EG.UTF-8 LC_ALL=` and `export LANGUAGE=en:en_US LANG=en_US.UTF-8 LC_ALL=` environments."
            }
        }
    }
}
