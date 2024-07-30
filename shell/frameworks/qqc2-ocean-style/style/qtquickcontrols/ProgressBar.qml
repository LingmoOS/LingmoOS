/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.ProgressBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false

    contentItem: Item {
        implicitWidth: 200
        implicitHeight: Impl.Units.grooveHeight
        clip: true
        Rectangle {
            id: progressFill
            visible: !control.indeterminate && width > 0
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: control.position * parent.width

            radius: Impl.Units.grooveHeight/2
            color: LingmoUI.Theme.alternateBackgroundColor
            border {
                width: Impl.Units.smallBorder
                color: LingmoUI.Theme.focusColor
            }
        }

        Item {
            id: indeterminateFill

            readonly property real __backgroundBorderWidth: control.background?.border?.width ?? 0

            anchors {
                top: parent.top
                bottom: parent.bottom
                topMargin: __backgroundBorderWidth
                bottomMargin: __backgroundBorderWidth
            }
            width: parent.width + 2 * __segmentLength
            x: - 2 * __segmentLength

            clip: true
            visible: control.indeterminate && width > 0

            readonly property real __segmentLength: 14

            Row {
                anchors.fill: parent
                spacing: indeterminateFill.__segmentLength

                Repeater {
                    model: Math.round(parent.width / (2 * indeterminateFill.__segmentLength))
                    delegate: Rectangle {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        implicitWidth: indeterminateFill.__segmentLength
                        radius: Impl.Units.grooveHeight/2
                        color: LingmoUI.Theme.alternateBackgroundColor
                    }
                }
            }

            XAnimator on x {
                from: - 2 * indeterminateFill.__segmentLength
                to: 0
                duration: 3 * LingmoUI.Units.veryLongDuration
                loops: Animation.Infinite
                running: true
            }
        }
    }

    background: Rectangle {
        implicitWidth: 200
        implicitHeight: Impl.Units.grooveHeight

        radius: Impl.Units.grooveHeight/2
        color: LingmoUI.Theme.backgroundColor
        border {
            width: Impl.Units.smallBorder
            color: Impl.Theme.separatorColor()
        }
    }
}
