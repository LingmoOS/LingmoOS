/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 ProgressBar"
    property int progressBarWidth: testProgressBar.width

    LingmoComponents.ProgressBar {
        id: testProgressBar
        visible: false
    }

    contentItem: GridLayout {
        columns: 6
        columnSpacing: LingmoUI.Units.gridUnit
        rowSpacing: LingmoUI.Units.gridUnit

        ColumnLayout {
            LingmoComponents.Label {
                text: "0%"
            }
            LingmoComponents.ProgressBar {
                from: 0
                to: 100
                value: 0
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "50%"
            }
            LingmoComponents.ProgressBar {
                from: 0
                to: 100
                value: 50
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "100%"
            }
            LingmoComponents.ProgressBar {
                from: 0
                to: 100
                value: 100
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                id: progressBarAndSliderLabel
                text: "The progress bar and slider grooves should have the same visual width."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            GridLayout {
                id: progressBarAndSliderGrid
                columns: 1
                rows: 2
                LingmoComponents.ProgressBar {
                    id: progressBar
                    from: 0
                    to: 100
                    value: 50
                }
                LingmoComponents.Slider {
                    Layout.preferredWidth: progressBar.width
                     Layout.preferredHeight: progressBar.height
                    from: 0
                    to: 100
                    value: 50
                }
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "Min: 0; Max: 200; Value: 1\nMake sure the bar does not leak outside."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            LingmoComponents.ProgressBar {
                from: 0
                to: 200
                value: 1
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "Min: 0; Max: 100; Value: 110\nThe progress bar should look like it is at 100%."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            LingmoComponents.ProgressBar {
                from: 0
                to: 100
                value: 110
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "Min: -100; Max: 100; Value: 0\nThe progress bar should look like it is at 50%."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            LingmoComponents.ProgressBar {
                from: -100
                to: 100
                value: 0
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "Min: 0; Max: 100; Value: -10\nThe progress bar should look like it is at 0%."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            LingmoComponents.ProgressBar {
                from: 0
                to: 100
                value: -10
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "This should have a continuous movement from one end to the other and back."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            LingmoComponents.ProgressBar {
                indeterminate: indeterminateCheckBox.checked
                value: 0.5
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "Checking and unchecking should not break the layout. The progress bar should look like it is at 50% if unchecked."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            LingmoComponents.CheckBox {
                id: indeterminateCheckBox
                text: "Indeterminate"
                checked: true
            }
        }

        ColumnLayout {
            LingmoComponents.Label {
                text: "This should do one 'indefinite' animation cycle and then continuously animate to 100% in chunks of 10%."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            LingmoComponents.ProgressBar {
                id: animatingProgressBar
                from: 0
                to: 100
                // Bug 430544: A ProgressBar that was indeterminate once will
                // not update its bar size according to its value anymore
                // Set to false again in the Timer below
                indeterminate: true

                Timer {
                    interval: 500
                    triggeredOnStart: true
                    running: true
                    repeat: true
                    onTriggered: {
                        animatingProgressBar.indeterminate = false;

                        // ProgressBar clamps "value" by "to" (100), so we can't
                        // just blindly increase and then check >= 100
                        if (animatingProgressBar.value === 100) {
                            animatingProgressBar.value = 0;
                        } else {
                            animatingProgressBar.value += 10;
                        }
                    }
                }
            }
        }

    }
}
