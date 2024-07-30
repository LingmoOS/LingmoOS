/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQml
import QtQuick.Templates as T
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

QQC2.ApplicationWindow {
    id: root
    width: 200 * Qt.styleHints.wheelScrollLines + scrollView.leftPadding + scrollView.rightPadding
    height: 200 * Qt.styleHints.wheelScrollLines + scrollView.topPadding + scrollView.bottomPadding
    visible: true
    ScrollView {
        id: scrollView
        anchors.fill: parent
        Grid {
            columns: Math.sqrt(visibleChildren.length)
            Repeater {
                model: 500
                delegate: Rectangle {
                    implicitWidth: 20 * Qt.styleHints.wheelScrollLines
                    implicitHeight: 20 * Qt.styleHints.wheelScrollLines
                    gradient: Gradient {
                        orientation: index % 2 ? Gradient.Vertical : Gradient.Horizontal
                        GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                        GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    }
                }
            }
            QQC2.Button {
                id: enableSliderButton
                width: 20 * Qt.styleHints.wheelScrollLines
                height: 20 * Qt.styleHints.wheelScrollLines
                contentItem: QQC2.Label {
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: "Enable Slider"
                    wrapMode: Text.Wrap
                }
                checked: true
            }
            QQC2.Slider {
                id: slider
                enabled: enableSliderButton.checked
                width: 20 * Qt.styleHints.wheelScrollLines
                height: 20 * Qt.styleHints.wheelScrollLines
            }
            Repeater {
                model: 500
                delegate: Rectangle {
                    implicitWidth: 20 * Qt.styleHints.wheelScrollLines
                    implicitHeight: 20 * Qt.styleHints.wheelScrollLines
                    gradient: Gradient {
                        orientation: index % 2 ? Gradient.Vertical : Gradient.Horizontal
                        GradientStop { position: 0; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                        GradientStop { position: 1; color: Qt.rgba(Math.random(),Math.random(),Math.random(),1) }
                    }
                }
            }
        }
    }
}
