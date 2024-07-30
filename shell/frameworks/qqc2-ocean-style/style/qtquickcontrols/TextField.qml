// NOTE: checkthis
/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean.impl as Impl

T.TextField {
    id: control

    property bool visualFocus: control.activeFocus && (
        control.focusReason == Qt.TabFocusReason ||
        control.focusReason == Qt.BacktabFocusReason ||
        control.focusReason == Qt.ShortcutFocusReason
    )

    implicitWidth: implicitBackgroundWidth + leftInset + rightInset
                   || Math.ceil(Math.max(contentWidth, placeholder.implicitWidth)) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    padding: LingmoUI.Units.mediumSpacing
    leftPadding: Impl.Units.mediumHorizontalPadding
    rightPadding: Impl.Units.mediumHorizontalPadding

    LingmoUI.Theme.colorSet: LingmoUI.Theme.View
    LingmoUI.Theme.inherit: !background || !background.visible

    color: LingmoUI.Theme.textColor
    selectionColor: LingmoUI.Theme.highlightColor
    selectedTextColor: LingmoUI.Theme.highlightedTextColor
    placeholderTextColor: LingmoUI.Theme.disabledTextColor
    verticalAlignment: TextInput.AlignVCenter
    horizontalAlignment: TextInput.AlignLeft

    selectByMouse: true
    mouseSelectionMode: LingmoUI.Settings.tabletMode ?
        TextInput.SelectWords : TextInput.SelectCharacters

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

    background: Impl.TextEditBackground {
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
        if (LingmoUI.Settings.tabletMode) {
            forceActiveFocus();
            cursorPosition = positionAt(event.x, event.y);
            selectWord();
            mobileTextActionsToolBar.item.open()
        }
    }
}
