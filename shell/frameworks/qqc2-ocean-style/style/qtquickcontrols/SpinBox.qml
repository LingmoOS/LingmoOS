/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.SpinBox {
    id: control
    property real __downIndicatorWidth: down.indicator ? down.indicator.width : 0
    property real __upIndicatorWidth: up.indicator ? up.indicator.width : 0
    property real __leftIndicatorWidth: control.mirrored ? __upIndicatorWidth : __downIndicatorWidth
    property real __rightIndicatorWidth: control.mirrored ? __downIndicatorWidth : __upIndicatorWidth

    implicitWidth: Math.max(
        implicitBackgroundWidth + leftInset + rightInset,
        implicitContentWidth + leftPadding + rightPadding,
        up.implicitIndicatorWidth + down.implicitIndicatorWidth
    )
    implicitHeight: Math.max(
        implicitBackgroundHeight + topInset + bottomInset,
        implicitContentHeight + topPadding + bottomPadding,
        up.implicitIndicatorHeight,
        down.implicitIndicatorHeight
    )

    hoverEnabled: Qt.styleHints.useHoverEffects

    LingmoUI.Theme.colorSet: control.editable ? LingmoUI.Theme.View : LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: !Boolean(background)

    editable: true
    inputMethodHints: Qt.ImhDigitsOnly
    wheelEnabled: true

    padding: LingmoUI.Units.mediumSpacing
    leftPadding: __leftIndicatorWidth
    rightPadding: __rightIndicatorWidth
    spacing: LingmoUI.Units.mediumSpacing

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    down.indicator: Impl.SpinBoxIndicator {
        button: control.down
        alignment: Qt.AlignLeft
        mirrored: control.mirrored
    }

    /* NOTE: There is a flaw in QQC2 SpinBox with how focus is handled.
     * Unless you make the TextInput unfocusable (not what you want),
     * you can't detect whether or not a spinbox has activeFocus or visualFocus.
     * This is because the TextInput takes the focus.
     * In order to detect if a SpinBox is focused, you must trust that the
     * contentItem will be the part that takes the focus.
     */

    contentItem: Controls.TextField {
        palette: control.palette
        leftPadding: control.spacing
        rightPadding: control.spacing
        topPadding: 0
        bottomPadding: 0
        // Intentionally using anchors so that left/right
        // control padding can be used like it normally would
        text: control.displayText
        font: control.font
        color: LingmoUI.Theme.textColor
        selectionColor: LingmoUI.Theme.highlightColor
        selectedTextColor: LingmoUI.Theme.highlightedTextColor
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter

        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        selectByMouse: true // Should this be disabled for mobile?
        background: null

        // Since the contentItem receives focus (we make them editable by default),
        // the screen reader reads its Accessible properties instead of the SpinBox's
        Accessible.name: control.Accessible.name
        Accessible.description: control.Accessible.description
    }

    up.indicator: Impl.SpinBoxIndicator {
        button: control.up
        alignment: Qt.AlignRight
        mirrored: control.mirrored
    }

    background:Impl.TextEditBackground {
        control: control
        implicitWidth: Impl.Units.mediumControlHeight * 3 - Impl.Units.smallBorder * 2
        implicitHeight: Impl.Units.mediumControlHeight
        // Work around SpinBox focus handling flaw
        visualFocus: control.visualFocus || (control.contentItem.activeFocus && (
            control.contentItem.focusReason == Qt.TabFocusReason ||
            control.contentItem.focusReason == Qt.BacktabFocusReason ||
            control.contentItem.focusReason == Qt.ShortcutFocusReason
        ))
    }
}
