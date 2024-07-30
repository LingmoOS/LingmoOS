// NOTE: check this
/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.TextArea {
    id: control

    implicitWidth: Math.max(contentWidth + leftPadding + rightPadding,
                            implicitBackgroundWidth + leftInset + rightInset,
                            placeholder.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                             implicitBackgroundHeight + topInset + bottomInset,
                             placeholder.implicitHeight + topPadding + bottomPadding)
    property bool visualFocus: control.activeFocus && (
        control.focusReason == Qt.TabFocusReason ||
        control.focusReason == Qt.BacktabFocusReason ||
        control.focusReason == Qt.ShortcutFocusReason
    )


    padding: LingmoUI.Units.mediumSpacing
    property real horizontalPadding: Impl.Units.mediumHorizontalPadding
    property real verticalPadding: padding
    leftPadding: horizontalPadding
    rightPadding: horizontalPadding
    topPadding: verticalPadding
    bottomPadding: verticalPadding

    LingmoUI.Theme.colorSet: LingmoUI.Theme.View
    LingmoUI.Theme.inherit: background == null

    color: LingmoUI.Theme.textColor
    selectionColor: LingmoUI.Theme.highlightColor
    selectedTextColor: LingmoUI.Theme.highlightedTextColor
    placeholderTextColor: LingmoUI.Theme.disabledTextColor

    selectByMouse: true
    mouseSelectionMode: LingmoUI.Settings.tabletMode ?
        TextEdit.SelectWords : TextEdit.SelectCharacters

    cursorDelegate: Loader {
        visible: control.activeFocus && !control.readOnly && control.selectionStart === control.selectionEnd
        active: visible
        sourceComponent: Impl.CursorDelegate { target: control }
    }

    Controls.Label {
        id: placeholder
        anchors {
            fill: parent
            leftMargin: control.leftPadding
            rightMargin: control.rightPadding
            topMargin: control.topPadding
            bottomMargin: control.bottomPadding
        }

        text: control.placeholderText
        font: control.font
        color: control.placeholderTextColor
        horizontalAlignment: control.horizontalAlignment
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
        renderType: control.renderType
    }

    background:Impl.TextEditBackground {
        control: control
        implicitWidth: 200
        visualFocus: control.visualFocus
    }

    Impl.CursorHandle {
        id: selectionStartHandle
        target: control
    }

    Impl.CursorHandle {
        id: selectionEndHandle
        target: control
        isSelectionEnd: true
    }

    MobileTextActionsToolBar {
        id: mobileTextActionsToolBar
        target: control
    }

    onActiveFocusChanged: {
        if (!activeFocus) {
            mobileTextActionsToolBar.visible = false
        } else if (LingmoUI.Settings.tabletMode) {
            mobileTextActionsToolBar.visible = true
        }
    }

    onSelectedTextChanged: {
        if (LingmoUI.Settings.tabletMode && selectedText.length > 0) {
            mobileTextActionsToolBar.item.open()
        }
    }

    onPressAndHold: {
        if (LingmoUI.Settings.tabletMode && selectByMouse) {
            forceActiveFocus();
            cursorPosition = positionAt(event.x, event.y);
            selectWord();
            mobileTextActionsToolBar.item.open()
        }
    }
}
