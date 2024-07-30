/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

LingmoUI.BasicThemeDefinition {
    textColor: palette.windowText
    disabledTextColor: disabledPalette.windowText

    highlightColor: palette.highlight
    highlightedTextColor: palette.highlightedText
    backgroundColor: palette.window
    alternateBackgroundColor: Qt.darker(palette.window, 1.05)
    activeTextColor: palette.highlight
    activeBackgroundColor: palette.highlight
    secondBackgroundColor: darkMode ? "#2C2C2D" : "#FFFFFF"
    bgkColor: "#000000"
    linkColor: "#2980B9"
    linkBackgroundColor: "#2980B9"
    visitedLinkColor: "#7F8C8D"
    visitedLinkBackgroundColor: "#7F8C8D"
    buttonColorT: "#0059DF"
    buttonCOlorB: "#2671E0"
    hoverColor: palette.highlight
    focusColor: palette.highlight
    negativeTextColor: "#DA4453"
    negativeBackgroundColor: "#DA4453"
    neutralTextColor: "#F67400"
    neutralBackgroundColor: "#F67400"
    positiveTextColor: "#27AE60"
    positiveBackgroundColor: "#27AE60"

    buttonTextColor: palette.buttonText
    buttonBackgroundColor: palette.button
    buttonAlternateBackgroundColor: Qt.darker(palette.button, 1.05)
    buttonHoverColor: palette.highlight
    buttonFocusColor: palette.highlight

    property var renderType: Text.QtRendering

    viewTextColor: palette.text
    viewBackgroundColor: palette.base
    viewAlternateBackgroundColor: palette.alternateBase
    viewHoverColor: palette.highlight
    viewFocusColor: palette.highlight

    selectionTextColor: palette.highlightedText
    selectionBackgroundColor: palette.highlight
    selectionAlternateBackgroundColor: Qt.darker(palette.highlight, 1.05)
    selectionHoverColor: palette.highlight
    selectionFocusColor: palette.highlight

    tooltipTextColor: palette.base
    tooltipBackgroundColor: palette.text
    tooltipAlternateBackgroundColor: Qt.darker(palette.text, 1.05)
    tooltipHoverColor: palette.highlight
    tooltipFocusColor: palette.highlight

    complementaryTextColor: palette.base
    complementaryBackgroundColor: palette.text
    complementaryAlternateBackgroundColor: Qt.darker(palette.text, 1.05)
    complementaryHoverColor: palette.highlight
    complementaryFocusColor: palette.highlight

    headerTextColor: palette.text
    headerBackgroundColor: palette.base
    headerAlternateBackgroundColor: palette.alternateBase
    headerHoverColor: palette.highlight
    headerFocusColor: palette.highlight

    property font defaultFont: fontMetrics.font

    property real fontSize: 20
    property string fontFamily: "Noto Sans"

    smallRadius: 8.0
    mediumRadius: 10.0
    bigRadius: 12.0
    hugeRadius: 14.0
    windowRadius: 11.0

    property list<QtObject> children: [
        TextMetrics {
            id: fontMetrics
        },
        SystemPalette {
            id: palette
            colorGroup: SystemPalette.Active
        },
        SystemPalette {
            id: disabledPalette
            colorGroup: SystemPalette.Disabled
        }
    ]

    function __propagateColorSet(object, context) {}

    function __propagateTextColor(object, color) {}
    function __propagateBackgroundColor(object, color) {}
    function __propagatePrimaryColor(object, color) {}
    function __propagateAccentColor(object, color) {}
}
