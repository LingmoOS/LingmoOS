/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

QQC2.Pane {
    id: root

    width: 800
    height: 100
    padding: 0

    component Guide : Kirigami.Separator {
        anchors.left: parent.left
        anchors.right: parent.right
        opacity: 0.7
        z: 1
    }

    QQC2.ScrollView {
        anchors.fill: parent

        Kirigami.Padding {
            padding: Kirigami.Units.largeSpacing

            Guide {
                anchors.top: layout.top
            }

            Guide {
                anchors.bottom: layout.bottom
            }

            Guide {
                anchors.verticalCenter: parent.verticalCenter
            }

            Guide {
                anchors.top: layout.top
                anchors.topMargin: checkBox.y + checkBox.baselineOffset
            }

            contentItem: RowLayout {
                id: layout

                spacing: Kirigami.Units.smallSpacing

                QQC2.CheckBox {
                    id: checkBox
                    Layout.alignment: Qt.AlignBaseline
                    text: "Hello"
                }
                QQC2.Button {
                    Layout.alignment: Qt.AlignBaseline
                    text: "Hello"
                }
                QQC2.ToolButton {
                    Layout.alignment: Qt.AlignBaseline
                    text: "Hello"
                }
                QQC2.DelayButton {
                    Layout.alignment: Qt.AlignBaseline
                    text: "Hello"
                }
                QQC2.RadioButton {
                    Layout.alignment: Qt.AlignBaseline
                    text: "Hello"
                }
                QQC2.Slider {
                    Layout.alignment: Qt.AlignBaseline
                    Layout.preferredWidth: 50
                    value: 0.7
                }
                QQC2.SpinBox {
                    Layout.alignment: Qt.AlignBaseline
                }
                QQC2.ComboBox {
                    Layout.alignment: Qt.AlignBaseline
                    model: ["Hello"]
                }
                QQC2.TextField {
                    Layout.alignment: Qt.AlignBaseline
                    Layout.preferredWidth: 50
                    text: "Hello"
                }
                QQC2.TextArea {
                    Layout.alignment: Qt.AlignBaseline
                    Layout.preferredWidth: 50
                    text: "Hello"
                }
                QQC2.Switch {
                    Layout.alignment: Qt.AlignBaseline
                    text: "Hello"
                }
                QQC2.ProgressBar {
                    Layout.alignment: Qt.AlignBaseline
                    Layout.preferredWidth: 50
                    value: 0.7
                }
            }
        }
    }
}
