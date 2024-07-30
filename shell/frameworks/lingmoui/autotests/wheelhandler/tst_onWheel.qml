/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    id: root
    name: "WheelHandler onWheel"
    visible: true
    when: windowShown
    width: 600
    height: 600

    function test_onWheel() {
        let contentX = flickable.contentX
        let contentY = flickable.contentY
        let contentWidth = flickable.contentWidth
        let contentHeight = flickable.contentHeight

        // grow
        mouseWheel(flickable, flickable.leftMargin, 0, -120, -120, Qt.NoButton, Qt.ControlModifier)
        verify(flickable.contentWidth === contentWidth - 120, "-xDelta")
        contentWidth = flickable.contentWidth
        verify(flickable.contentHeight === contentHeight - 120, "-yDelta")
        contentHeight = flickable.contentHeight

        // check if accepting the event prevents scrolling
        verify(flickable.contentX === contentX && flickable.contentY === contentY, "not moved")

        // shrink
        mouseWheel(flickable, flickable.leftMargin, 0, 120, 120, Qt.NoButton, Qt.ControlModifier)
        verify(flickable.contentWidth === contentWidth + 120, "+xDelta")
        verify(flickable.contentHeight === contentHeight + 120, "+yDelta")

        // check if accepting the event prevents scrolling
        verify(flickable.contentX === contentX && flickable.contentY === contentY, "not moved")
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Flickable {
        id: flickable
        anchors.fill: parent
        LingmoUI.WheelHandler {
            id: wheelHandler
            target: flickable
            onWheel: {
                if (wheel.modifiers & Qt.ControlModifier) {
                    // Adding delta is the simplest way to change size without running into floating point number issues
                    // NOTE: Not limiting minimum content size to a size greater than the Flickable size makes it so
                    // wheel events stop coming to onWheel when the content size is the size of the flickable or smaller.
                    // Maybe it's a Flickable issue? Koko had the same problem with Flickable.
                    flickable.contentWidth = Math.max(720, flickable.contentWidth + wheel.angleDelta.x)
                    flickable.contentHeight = Math.max(720, flickable.contentHeight + wheel.angleDelta.y)
                    wheel.accepted = true
                }
            }
        }
        leftMargin: QQC2.ScrollBar.vertical && QQC2.ScrollBar.vertical.visible && QQC2.ScrollBar.vertical.mirrored ? QQC2.ScrollBar.vertical.width : 0
        rightMargin: QQC2.ScrollBar.vertical && QQC2.ScrollBar.vertical.visible && !QQC2.ScrollBar.vertical.mirrored ? QQC2.ScrollBar.vertical.width : 0
        bottomMargin: QQC2.ScrollBar.horizontal && QQC2.ScrollBar.horizontal.visible ? QQC2.ScrollBar.horizontal.height : 0
        QQC2.ScrollBar.vertical: QQC2.ScrollBar {
            parent: flickable.parent
            anchors.right: flickable.right
            anchors.top: flickable.top
            anchors.bottom: flickable.bottom
            anchors.bottomMargin: flickable.QQC2.ScrollBar.horizontal ? flickable.QQC2.ScrollBar.horizontal.height : anchors.margins
            active: flickable.QQC2.ScrollBar.vertical.active
        }
        QQC2.ScrollBar.horizontal: QQC2.ScrollBar {
            parent: flickable.parent
            anchors.left: flickable.left
            anchors.right: flickable.right
            anchors.bottom: flickable.bottom
            anchors.rightMargin: flickable.QQC2.ScrollBar.vertical ? flickable.QQC2.ScrollBar.vertical.width : anchors.margins
            active: flickable.QQC2.ScrollBar.horizontal.active
        }
        contentWidth: 1200
        contentHeight: 1200
        Rectangle {
            id: contentRect
            anchors.fill: parent
            gradient: Gradient.WideMatrix
            border.color: Qt.rgba(0,0,0,0.5)
            border.width: 2
        }
    }

    QQC2.Label {
        anchors.centerIn: parent
        leftPadding: 4
        rightPadding: 4
        wrapMode: Text.Wrap
        color: "white"
        text: `Rectangle size: ${contentRect.width}x${contentRect.height}`
        background: Rectangle {
            color: "black"
        }
    }
}
