// SPDX-FileCopyrightText: 2022 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import org.kde.lingmoui as LingmoUI

Rectangle {

    /**
     * @brief This property holds the chip's default background color.
     */
    property color defaultColor: LingmoUI.Theme.backgroundColor

    /**
     * @brief This property holds the color of the Chip's background when it is being pressed.
     * @see QtQuick.AbstractButton::down
     */
    property color pressedColor: Qt.rgba(LingmoUI.Theme.highlightColor.r, LingmoUI.Theme.highlightColor.g, LingmoUI.Theme.highlightColor.b, 0.3)

    /**
     * @brief This property holds the color of the Chip's background when it is checked.
     * @see QtQuick.AbstractButton::checked
     */
    property color checkedColor: Qt.rgba(LingmoUI.Theme.highlightColor.r, LingmoUI.Theme.highlightColor.g, LingmoUI.Theme.highlightColor.b, 0.2)

    /**
     * @brief This property holds the chip's default border color.
     */
    property color defaultBorderColor: LingmoUI.ColorUtils.linearInterpolation(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, LingmoUI.Theme.frameContrast)

    /**
     * @brief This property holds the color of the Chip's border when it is checked.
     * @see QtQuick.AbstractButton::checked
     */
    property color checkedBorderColor: Qt.rgba(LingmoUI.Theme.highlightColor.r, LingmoUI.Theme.highlightColor.g, LingmoUI.Theme.highlightColor.b, 0.9)

    /**
     * @brief This property holds the color of the Chip's border when it is being pressed.
     * @see QtQuick.AbstractButton::down
     */
    property color pressedBorderColor: Qt.rgba(LingmoUI.Theme.highlightColor.r, LingmoUI.Theme.highlightColor.g, LingmoUI.Theme.highlightColor.b, 0.7)

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Header
    LingmoUI.Theme.inherit: false

    color: {
        if (parent.down) {
            return pressedColor
        } else if (parent.checked) {
            return checkedColor
        } else {
            return defaultColor
        }
    }
    border.color: {
        if (parent.down) {
            return pressedBorderColor
        } else if (parent.checked) {
            return checkedBorderColor
        } else {
            return defaultBorderColor
        }
    }
    border.width: 1
    radius: LingmoUI.Units.cornerRadius
}
