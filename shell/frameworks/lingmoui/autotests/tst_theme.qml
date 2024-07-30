/*
 *  SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: testCase
    name: "ThemeTest"

    width: 400
    height: 400
    visible: true

    when: windowShown

    TextMetrics {
        id: textMetrics
    }

    // Not all properties are updated immediately to avoid having massive storms
    // of duplicated signals and to prevent changes from retriggering code that
    // changed it. To deal with that, we need to wait a bit before continuiing
    // when we change properties. This time shouldn't be too short because on
    // some machines it may take a bit longer for things to properly be updated.
    function waitForEvents()
    {
        wait(20)
    }

    Component {
        id: basic

        Rectangle {
            color: LingmoUI.Theme.backgroundColor

            property alias text: textItem

            Text {
                id: textItem
                color: LingmoUI.Theme.textColor
                font: LingmoUI.Theme.defaultFont
            }
        }
    }

    function test_basic() {
        var item = createTemporaryObject(basic, testCase)
        verify(item)

        compare(item.LingmoUI.Theme.colorSet, LingmoUI.Theme.Window)
        compare(item.LingmoUI.Theme.colorGroup, LingmoUI.Theme.Active)
        verify(item.LingmoUI.Theme.inherit)

        compare(item.color, "#eff0f1")
        compare(item.text.color, "#31363b")
        compare(item.text.font.family, textMetrics.font.family)
    }

    Component {
        id: override

        Rectangle {
            LingmoUI.Theme.backgroundColor: "#ff0000"
            color: LingmoUI.Theme.backgroundColor
        }
    }

    function test_override() {
        var item = createTemporaryObject(override, testCase)
        verify(item)

        compare(item.color, "#ff0000")

        item.LingmoUI.Theme.backgroundColor = "#00ff00"

        // Changes to Theme are not immediately propagated, so give it a few
        // moments.
        waitForEvents()

        compare(item.color, "#00ff00")

        // Changing colorSet or colorGroup does not affect local overrides
        item.LingmoUI.Theme.colorSet = LingmoUI.Theme.Complementary
        item.LingmoUI.Theme.colorGroup = LingmoUI.Theme.Disabled

        waitForEvents()

        compare(item.color, "#00ff00")
    }

    Component {
        id: inherit

        Rectangle {
            color: LingmoUI.Theme.backgroundColor

            property alias child1: rect1
            property alias child2: rect2

            Rectangle {
                id: rect1
                color: LingmoUI.Theme.backgroundColor
            }
            Rectangle {
                id: rect2
                LingmoUI.Theme.inherit: false
                color: LingmoUI.Theme.backgroundColor
            }
        }
    }

    function test_inherit() {
        var item = createTemporaryObject(inherit, testCase)
        verify(item)

        // Default values are all the same
        compare(item.color, "#eff0f1")
        compare(item.child1.color, "#eff0f1")
        compare(item.child2.color, "#eff0f1")

        // If we change the colorSet, the item that inherits gets updated, but
        // the item that does not stays the same.
        item.LingmoUI.Theme.colorSet = LingmoUI.Theme.View

        waitForEvents()

        compare(item.color, "#fcfcfc")
        compare(item.child1.color, "#fcfcfc")
        compare(item.child2.color, "#eff0f1")

        // If we override a color, the item that inherits gets that color, while
        // the item that does not ignores it.
        item.LingmoUI.Theme.backgroundColor = "#ff0000"

        waitForEvents()

        compare(item.color, "#ff0000")
        compare(item.child1.color, "#ff0000")
        compare(item.child2.color, "#eff0f1")

        // If we change the color set again, the overridden color remains the
        // same for both the original object and the inherited object.
        item.LingmoUI.Theme.colorSet = LingmoUI.Theme.View

        waitForEvents()

        compare(item.color, "#ff0000")
        compare(item.child1.color, "#ff0000")
        compare(item.child2.color, "#eff0f1")

        // If we override a color of the item that inherits, it will stay the
        // same even if that color changes in the parent.
        item.child1.LingmoUI.Theme.backgroundColor = "#00ff00"
        item.LingmoUI.Theme.backgroundColor = "#0000ff"

        waitForEvents()

        compare(item.color, "#0000ff")
        compare(item.child1.color, "#00ff00")
        compare(item.child2.color, "#eff0f1")
    }

    Component {
        id: deepInherit

        Rectangle {
            color: LingmoUI.Theme.backgroundColor

            property alias child1: rect1
            property alias child2: rect2
            property alias child3: rect3

            Rectangle {
                id: rect1
                color: LingmoUI.Theme.backgroundColor

                Rectangle {
                    id: rect2
                    color: LingmoUI.Theme.backgroundColor

                    Rectangle {
                        id: rect3
                        color: LingmoUI.Theme.backgroundColor
                    }
                }
            }
        }
    }

    function test_inherit_deep() {
        var item = createTemporaryObject(deepInherit, testCase)
        verify(item)

        waitForEvents()

        compare(item.color, "#eff0f1")
        compare(item.child1.color, "#eff0f1")
        compare(item.child2.color, "#eff0f1")
        compare(item.child3.color, "#eff0f1")

        item.LingmoUI.Theme.backgroundColor = "#ff0000"

        waitForEvents()

        compare(item.color, "#ff0000")
        compare(item.child1.color, "#ff0000")
        compare(item.child2.color, "#ff0000")
        compare(item.child3.color, "#ff0000")

        item.child2.LingmoUI.Theme.inherit = false
        item.child2.LingmoUI.Theme.backgroundColor = "#00ff00"

        waitForEvents()

        compare(item.color, "#ff0000")
        compare(item.child1.color, "#ff0000")
        compare(item.child2.color, "#00ff00")
        compare(item.child3.color, "#00ff00")

        item.child2.LingmoUI.Theme.inherit = true
        item.child2.LingmoUI.Theme.backgroundColor = undefined

        waitForEvents()

        compare(item.color, "#ff0000")
        compare(item.child1.color, "#ff0000")
        compare(item.child2.color, "#ff0000")
        compare(item.child3.color, "#ff0000")

        item.child2.LingmoUI.Theme.colorSet = LingmoUI.Theme.Complementary
        item.child2.LingmoUI.Theme.inherit = false

        waitForEvents()

        compare(item.color, "#ff0000")
        compare(item.child1.color, "#ff0000")
        compare(item.child2.color, "#31363b")
        compare(item.child3.color, "#31363b")
    }

    Component {
        id: colorSet

        Rectangle {
            LingmoUI.Theme.colorSet: LingmoUI.Theme.View
            color: LingmoUI.Theme.backgroundColor
        }
    }

    function test_colorset() {
        var item = createTemporaryObject(colorSet, testCase)
        verify(item)

        waitForEvents()

        compare(item.color, "#fcfcfc")

        item.LingmoUI.Theme.colorSet = LingmoUI.Theme.Complementary

        waitForEvents()

        compare(item.color, "#31363b")
    }

    Component {
        id: colorGroup

        Rectangle {
            LingmoUI.Theme.colorGroup: LingmoUI.Theme.Disabled
            color: LingmoUI.Theme.backgroundColor
        }
    }

    function test_colorGroup() {
        var item = createTemporaryObject(colorGroup, testCase)
        verify(item)

        waitForEvents()

        var color = Qt.tint("#eff0f1", "transparent")

        compare(item.color, Qt.hsva(color.hsvHue, color.hsvSaturation * 0.5, color.hsvValue * 0.8))

        item.LingmoUI.Theme.colorGroup = LingmoUI.Theme.Inactive

        waitForEvents()

        compare(item.color, Qt.hsva(color.hsvHue, color.hsvSaturation * 0.5, color.hsvValue))
    }

    Component {
        id: palette

        Rectangle {
            color: LingmoUI.Theme.backgroundColor

            property alias child: button

            Button {
                id: button
                palette: LingmoUI.Theme.palette
            }
        }
    }

    function test_palette() {
        skip("palette property type has changed in Qt 6, LingmoUI.Theme and this test case need adjustments")
        var item = createTemporaryObject(palette, testCase)
        verify(item)

        compare(item.child.background.color, "#eff0f1")
        compare(item.child.contentItem.color, "#31363b")

        item.LingmoUI.Theme.backgroundColor = "#ff0000"

        waitForEvents()

        compare(item.child.background.color, "#ff0000")
    }
}
