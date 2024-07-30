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

    name: "WheelHandler invokable functions"
    visible: true
    when: windowShown
    width: flickable.implicitWidth
    height: flickable.implicitHeight

    function test_Invokables() {
        const originalX = flickable.contentX
        const originalY = flickable.contentY
        let x = originalX
        let y = originalY

        wheelHandler.scrollRight()
        tryCompare(flickable, "contentX", x + hstep, LingmoUI.Units.longDuration * 2, "scrollRight()")
        x = flickable.contentX

        wheelHandler.scrollLeft()
        tryCompare(flickable, "contentX", x - hstep, LingmoUI.Units.longDuration * 2, "scrollLeft()")
        x = flickable.contentX

        wheelHandler.scrollDown()
        tryCompare(flickable, "contentY", y + vstep, LingmoUI.Units.longDuration * 2, "scrollDown()")
        y = flickable.contentY

        wheelHandler.scrollUp()
        tryCompare(flickable, "contentY", y - vstep, LingmoUI.Units.longDuration * 2, "scrollUp()")
        y = flickable.contentY

        wheelHandler.scrollRight(101)
        tryCompare(flickable, "contentX", x + 101, LingmoUI.Units.longDuration * 2, "scrollRight(101)")
        x = flickable.contentX

        wheelHandler.scrollLeft(101)
        tryCompare(flickable, "contentX", x - 101, LingmoUI.Units.longDuration * 2, "scrollLeft(101)")
        x = flickable.contentX

        wheelHandler.scrollDown(101)
        tryCompare(flickable, "contentY", y + 101, LingmoUI.Units.longDuration * 2, "scrollDown(101)")
        y = flickable.contentY

        wheelHandler.scrollUp(101)
        tryCompare(flickable, "contentY", y - 101, LingmoUI.Units.longDuration * 2, "scrollUp(101)")
        y = flickable.contentY
    }

    ScrollableFlickable {
        id: flickable
        anchors.fill: parent
        LingmoUI.WheelHandler {
            id: wheelHandler
            target: flickable
        }
    }
}

