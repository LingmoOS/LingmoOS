/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    id: root
    readonly property real hstep: wheelHandler.horizontalStepSize
    readonly property real vstep: wheelHandler.verticalStepSize
    readonly property real pageWidth: flickable.width - flickable.leftMargin - flickable.rightMargin
    readonly property real pageHeight: flickable.height - flickable.topMargin - flickable.bottomMargin
    readonly property real contentWidth: flickable.contentWidth
    readonly property real contentHeight: flickable.contentHeight
    property alias wheelHandler: wheelHandler
    property alias flickable: flickable

    name: "WheelHandler scrolling"
    visible: true
    when: windowShown
    width: flickable.implicitWidth
    height: flickable.implicitHeight

    function wheelScrolling(angleDelta = wheelHandler.verticalStepSize) {
        let x = flickable.contentX
        let y = flickable.contentY
        const angleDeltaFactor = angleDelta / 120
        mouseWheel(flickable, flickable.leftMargin, 0, -angleDelta, -angleDelta, Qt.NoButton)
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentX", Math.round(x + hstep * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "+xTick")
        } else {
            compare(flickable.contentX, Math.round(x + hstep * angleDeltaFactor), "+xTick")
        }
        x = flickable.contentX
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentY", Math.round(y + vstep * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "+yTick")
        } else {
            compare(flickable.contentY, Math.round(y + vstep * angleDeltaFactor), "+yTick")
        }
        y = flickable.contentY

        mouseWheel(flickable, flickable.leftMargin, 0, angleDelta, angleDelta, Qt.NoButton)
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentX", Math.round(x - hstep * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "-xTick")
        } else {
            compare(flickable.contentX, Math.round(x - hstep * angleDeltaFactor), "-xTick")
        }
        x = flickable.contentX
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentY", Math.round(y - vstep * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "-yTick")
        } else {
            compare(flickable.contentY, Math.round(y - vstep * angleDeltaFactor), "-yTick")
        }
        y = flickable.contentY

        if (Qt.platform.pluginName !== "xcb") {
            mouseWheel(flickable, flickable.leftMargin, 0, 0, -angleDelta, Qt.NoButton, Qt.AltModifier)
            tryCompare(flickable, "contentX", Math.round(x + hstep * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "+h_yTick")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, LingmoUI.Units.longDuration * 2, "no +yTick")

            mouseWheel(flickable, flickable.leftMargin, 0, 0, angleDelta, Qt.NoButton, Qt.AltModifier)
            tryCompare(flickable, "contentX", Math.round(x - hstep * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "-h_yTick")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, LingmoUI.Units.longDuration * 2, "no -yTick")
        }

        mouseWheel(flickable, flickable.leftMargin, 0, -angleDelta, -angleDelta, Qt.NoButton, wheelHandler.pageScrollModifiers)
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentX", Math.round(x + pageWidth * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "+xPage")
        } else {
            compare(flickable.contentX, Math.round(x + pageWidth * angleDeltaFactor), "+xPage")
        }
        x = flickable.contentX
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentY", Math.round(y + pageHeight * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "+yPage")
        } else {
            compare(flickable.contentY, Math.round(y + pageHeight * angleDeltaFactor), "+yPage")
        }
        y = flickable.contentY

        mouseWheel(flickable, flickable.leftMargin, 0, angleDelta, angleDelta, Qt.NoButton, wheelHandler.pageScrollModifiers)
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentX", Math.round(x - pageWidth * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "-xPage")
        } else {
            compare(flickable.contentX, Math.round(x - pageWidth * angleDeltaFactor), "-xPage")
        }
        x = flickable.contentX
        if (angleDelta === wheelHandler.verticalStepSize) {
            tryCompare(flickable, "contentY", Math.round(y - pageHeight * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "-yPage")
        } else {
            compare(flickable.contentY, Math.round(y - pageHeight * angleDeltaFactor), "-yPage")
        }
        y = flickable.contentY

        if (Qt.platform.pluginName !== "xcb") {
            mouseWheel(flickable, flickable.leftMargin, 0, 0, -angleDelta, Qt.NoButton,
                    Qt.AltModifier | wheelHandler.pageScrollModifiers)
            tryCompare(flickable, "contentX", Math.round(x + pageWidth * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "+h_yPage")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, LingmoUI.Units.longDuration * 2, "no +yPage")

            mouseWheel(flickable, flickable.leftMargin, 0, 0, angleDelta, Qt.NoButton,
                    Qt.AltModifier | wheelHandler.pageScrollModifiers)
            tryCompare(flickable, "contentX", Math.round(x - pageWidth * angleDeltaFactor), LingmoUI.Units.longDuration * 2, "-h_yPage")
            x = flickable.contentX
            tryCompare(flickable, "contentY", y, LingmoUI.Units.longDuration * 2, "no -yPage")
        }
    }

    function test_WheelScrolling() {
        // HID 1bcf:08a0 Mouse
        // Angle delta is 120, like most mice.
        wheelScrolling()
    }

    function test_HiResWheelScrolling() {
        // Logitech MX Master 3
        // Main wheel angle delta is at least 16, plus multiples of 8 when scrolling faster.
        wheelScrolling(16)
    }

    function test_TouchpadScrolling() {
        // UNIW0001:00 093A:0255 Touchpad
        // 2 finger scroll angle delta is at least 3, but larger increments are used when scrolling faster.
        wheelScrolling(3)
    }

    function keyboardScrolling() {
        const originalX = flickable.contentX
        const originalY = flickable.contentY
        let x = originalX
        let y = originalY
        keyClick(Qt.Key_Right)
        tryCompare(flickable, "contentX", x + hstep, LingmoUI.Units.longDuration * 2, "Key_Right")
        x = flickable.contentX

        keyClick(Qt.Key_Left)
        tryCompare(flickable, "contentX", x - hstep, LingmoUI.Units.longDuration * 2, "Key_Left")
        x = flickable.contentX

        keyClick(Qt.Key_Down)
        tryCompare(flickable, "contentY", y + vstep, LingmoUI.Units.longDuration * 2, "Key_Down")
        y = flickable.contentY

        keyClick(Qt.Key_Up)
        tryCompare(flickable, "contentY", y - vstep, LingmoUI.Units.longDuration * 2, "Key_Up")
        y = flickable.contentY

        keyClick(Qt.Key_PageDown)
        tryCompare(flickable, "contentY", y + pageHeight, LingmoUI.Units.longDuration * 2, "Key_PageDown")
        y = flickable.contentY

        keyClick(Qt.Key_PageUp)
        tryCompare(flickable, "contentY", y - pageHeight, LingmoUI.Units.longDuration * 2, "Key_PageUp")
        y = flickable.contentY

        keyClick(Qt.Key_End)
        tryCompare(flickable, "contentY", contentHeight - pageHeight, LingmoUI.Units.longDuration * 2, "Key_End")
        y = flickable.contentY

        keyClick(Qt.Key_Home)
        tryCompare(flickable, "contentY", originalY, LingmoUI.Units.longDuration * 2, "Key_Home")
        y = flickable.contentY

        keyClick(Qt.Key_PageDown, Qt.AltModifier)
        tryCompare(flickable, "contentX", x + pageWidth, LingmoUI.Units.longDuration * 2, "h_Key_PageDown")
        x = flickable.contentX

        keyClick(Qt.Key_PageUp, Qt.AltModifier)
        tryCompare(flickable, "contentX", x - pageWidth, LingmoUI.Units.longDuration * 2, "h_Key_PageUp")
        x = flickable.contentX

        keyClick(Qt.Key_End, Qt.AltModifier)
        tryCompare(flickable, "contentX", contentWidth - pageWidth, LingmoUI.Units.longDuration * 2, "h_Key_End")
        x = flickable.contentX

        keyClick(Qt.Key_Home, Qt.AltModifier)
        tryCompare(flickable, "contentX", originalX, LingmoUI.Units.longDuration * 2, "h_Key_Home")
    }

    function test_KeyboardScrolling() {
        keyboardScrolling()
    }

    function test_StepSize() {
        // 101 is a value unlikely to be used by any user's combination of settings and hardware
        wheelHandler.verticalStepSize = 101
        wheelHandler.horizontalStepSize = 101
        wheelScrolling()
        keyboardScrolling()
        // reset to default
        wheelHandler.verticalStepSize = undefined
        wheelHandler.horizontalStepSize = undefined
        verify(wheelHandler.verticalStepSize == 20 * Qt.styleHints.wheelScrollLines, "default verticalStepSize")
        verify(wheelHandler.horizontalStepSize == 20 * Qt.styleHints.wheelScrollLines, "default horizontalStepSize")
    }

    ScrollableFlickable {
        id: flickable
        focus: true
        anchors.fill: parent
        LingmoUI.WheelHandler {
            id: wheelHandler
            target: flickable
            keyNavigationEnabled: true
        }
    }
}
