/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Window
import org.kde.lingmoui as LingmoUI

pragma Singleton

/**
 * A set of values to define semantically sizes and durations
 * @inherit QtQuick.QtObject
 */
QtObject {
    id: units

    // The default border width
    property int smallBorder: 1
    // Used for the focus ring
    property int mediumBorder: smallBorder * 2
    // Usually for highlights on larger surfaces like Cards
    property int largeBorder: smallBorder * 4

    // The default corner radius
    property int smallRadius: LingmoUI.Units.cornerRadius
    // Usually for larger surfaces like Cards
    property int largeRadius: smallRadius * 2

    property int bigRadius: 12.0

    // Used to underline checkbox labels
    property int focusUnderlineThickness: smallBorder
    // Used for tabs and items in sidebars
    property int highlightLineThickness: smallRadius

    property int grooveHeight: {
        let h = Math.floor(fontMetrics.height / 3);
        h += h % 2;
        return h;
    }

    property int thickGrooveHeight: {
        let h = Math.floor(fontMetrics.height / 1.5);
        h += h % 2;
        return h;
    }

    /// For things like checkboxes/radiobuttons/switches/slider handles
    property int inlineControlHeight: fontMetrics.height

    // For small controls with a small amount of vertical padding
    property int smallControlHeight: fontMetrics.height + LingmoUI.Units.smallSpacing * 2

    // For medium controls with a medium amount of vertical padding
    property int mediumControlHeight: fontMetrics.height + LingmoUI.Units.mediumSpacing * 2

    // For large controls with a large amount of vertical padding
    property int largeControlHeight: fontMetrics.height + LingmoUI.Units.largeSpacing * 2

    property real horizontalPaddingRatio: Math.max(fontMetrics.height / fontMetrics.fullWidthCharWidth, 1)

    property int verySmallHorizontalPadding: Math.round(horizontalPaddingRatio * units.verySmallSpacing)

    property int smallHorizontalPadding: Math.round(horizontalPaddingRatio * LingmoUI.Units.smallSpacing)

    property int mediumHorizontalPadding: Math.round(horizontalPaddingRatio * LingmoUI.Units.mediumSpacing)

    property int largeHorizontalPadding: Math.round(horizontalPaddingRatio * LingmoUI.Units.largeSpacing)

    function symbolSize(size) {
        size -= size % 6
        size -= size/3
        return size
    }

    /**
     * Units.verySmallSpacing is the amount of spacing that should be used around smaller UI elements,
     * for example as spacing in Columns. Internally, this size depends on the size of
     * the default font as rendered on the screen, so it takes user-configured font size and DPI
     * into account.
     */
    property int verySmallSpacing: LingmoUI.Units.smallSpacing * 0.5

    /**
     * Units.veryLargeSpacing is the amount of spacing that should be used inside very big UI elements
     */
    property int veryLargeSpacing: LingmoUI.Units.largeSpacing * 1.5

    /**
     * metrics used by the default font
     */
    property var fontMetrics: FontMetrics {
        property real fullWidthCharWidth: fontMetrics.tightBoundingRect('＿').width
    }
}
