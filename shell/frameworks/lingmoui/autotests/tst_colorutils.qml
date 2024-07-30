/*
 *  SPDX-FileCopyrightText: 2023 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtTest
import org.kde.lingmoui as LingmoUI

TestCase {
    id: testCase
    name: "ColorUtilsTest"

    function test_linear_interpolation_data() {
        return [
            {tag: "black to white", from: Qt.rgba(0.0, 0.0, 0.0), to: Qt.rgba(1.0, 1.0, 1.0), amount: 0.5, expected: Qt.rgba(0.5, 0.5, 0.5)},
            {tag: "white to black", from: Qt.rgba(1.0, 1.0, 1.0), to: Qt.rgba(0.0, 0.0, 0.0), amount: 0.5, expected: Qt.rgba(0.5, 0.5, 0.5)},
            {tag: "red to white", from: Qt.rgba(1.0, 0.0, 0.0), to: Qt.rgba(1.0, 1.0, 1.0), amount: 0.5, expected: Qt.rgba(1.0, 0.5, 0.5)},
            {tag: "green to white", from: Qt.rgba(0.0, 0.0, 1.0), to: Qt.rgba(1.0, 1.0, 1.0), amount: 0.5, expected: Qt.rgba(0.5, 0.5, 1.0)},
            {tag: "transparent to black", from: Qt.rgba(0.0, 0.0, 0.0, 0.0), to: Qt.rgba(0.0, 0.0, 0.0), amount: 0.5, expected: Qt.rgba(0.0, 0.0, 0.0, 0.5)},
            {tag: "transparent to white", from: Qt.rgba(0.0, 0.0, 0.0, 0.0), to: Qt.rgba(1.0, 1.0, 1.0), amount: 0.5, expected: Qt.rgba(0.5, 0.5, 0.5, 0.5)},
            // This would produce a pink color when not accounting for undefined hue when using white.
            {tag: "broken separator", from: Qt.color("#af384c"), to: Qt.color("white"), amount: 0.15, expected: Qt.color("#bb4f61")}
        ]
    }

    function test_linear_interpolation(data) {
        let result = LingmoUI.ColorUtils.linearInterpolation(data.from, data.to, data.amount)
        let expected = data.expected

        // According to the documentation, fuzzyCompare() should be able to handle
        // color values but it seems to just do string comparison. So do a manual
        // component-wise comparison instead.
        fuzzyCompare(result.r, expected.r, 0.001, "Colors are not the same, Actual: " + result + " Expected: " + expected + ", component is red")
        fuzzyCompare(result.g, expected.g, 0.001, "Colors are not the same, Actual: " + result + " Expected: " + expected + ", component is green")
        fuzzyCompare(result.b, expected.b, 0.001, "Colors are not the same, Actual: " + result + " Expected: " + expected + ", component is blue")
        fuzzyCompare(result.a, expected.a, 0.001, "Colors are not the same, Actual: " + result + " Expected: " + expected + ", component is alpha")
    }
}

