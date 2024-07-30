/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI
import QtTest

TestCase {
    id: root
    name: "WheelHandler filterMouseEvents"
    visible: true
    when: windowShown
    width: flickable.implicitWidth
    height: flickable.implicitHeight

    function test_MouseFlick() {
        const x = flickable.contentX
        const y = flickable.contentY
        mousePress(flickable, flickable.leftMargin + 10, 10)
        mouseMove(flickable)
        mouseRelease(flickable)
        verify(flickable.contentX === x && flickable.contentY === y, "not moved")
    }

    // NOTE: Unfortunately, this test can't work. Flickable does not handle touch events, only mouse events synthesized from touch events
    // TODO: Uncomment if Flickable is ever able to use touch events.
    /*function test_TouchFlick() {
        const x = flickable.contentX, y = flickable.contentY
        let touch = touchEvent(flickable)
        // Press on center.
        touch.press(0, flickable)
        touch.commit()
        // Move a bit towards top left.
        touch.move(0, flickable, flickable.width/2 - 50, flickable.height/2 - 50)
        touch.commit()
        // Release at the spot we moved to.
        touch.release(0, flickable, flickable.width/2 - 50, flickable.height/2 - 50)
        touch.commit()
        verify(flickable.contentX !== x || flickable.contentY !== y, "moved")
    }*/

    function test_MouseScrollBars() {
        const x = flickable.contentX, y = flickable.contentY
        mousePress(flickable, flickable.leftMargin + 10, 10)
        mouseMove(flickable)
        const interactive = flickable.QQC2.ScrollBar.vertical.interactive || flickable.QQC2.ScrollBar.horizontal.interactive
        mouseRelease(flickable)
        verify(interactive, "interactive scrollbars")
    }

    function test_TouchScrollBars() {
        const x = flickable.contentX, y = flickable.contentY
        let touch = touchEvent(flickable)
        touch.press(0, flickable, flickable.leftMargin + 10, 10)
        touch.commit()
        touch.move(0, flickable)
        touch.commit()
        const interactive = flickable.QQC2.ScrollBar.vertical.interactive || flickable.QQC2.ScrollBar.horizontal.interactive
        touch.release(0, flickable)
        touch.commit()
        verify(!interactive, "no interactive scrollbars")
    }

    ScrollableFlickable {
        id: flickable
        anchors.fill: parent
        LingmoUI.WheelHandler {
            id: wheelHandler
            target: flickable
            filterMouseEvents: true
        }
    }
}
