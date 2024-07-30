/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Window
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.desktop.private as Private
import org.kde.qqc2desktopstyle.private as StylePrivate

T.TextField {
    id: controlRoot

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    implicitWidth: Math.max(200,
                            placeholderText ? placeholder.implicitWidth + leftPadding + rightPadding : 0)
                            || contentWidth + leftPadding + rightPadding
    implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                             background ? background.implicitHeight : 0,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    padding: 6

    placeholderTextColor: Kirigami.Theme.disabledTextColor
    color: enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
    selectionColor: Kirigami.Theme.highlightColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    verticalAlignment: TextInput.AlignVCenter
    hoverEnabled: !Kirigami.Settings.tabletMode

    selectByMouse: !Kirigami.Settings.tabletMode

    cursorDelegate: Kirigami.Settings.tabletMode ? mobileCursor : null

    Component {
        id: mobileCursor
        Private.MobileCursor {
            target: controlRoot
        }
    }

    onFocusChanged: {
        if (focus) {
            Private.MobileTextActionsToolBar.controlRoot = controlRoot;
        }
    }

    onTextChanged: Private.MobileTextActionsToolBar.shouldBeVisible = false;

    onPressed: event => Private.MobileTextActionsToolBar.shouldBeVisible = true;

    TapHandler {
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        // unfortunately, taphandler's pressed event only triggers when the press is lifted
        // we need to use the longpress signal since it triggers when the button is first pressed
        longPressThreshold: 0.001 // https://invent.kde.org/qt/qt/qtdeclarative/-/commit/8f6809681ec82da783ae8dcd76fa2c209b28fde6
        onLongPressed: {
            Private.TextFieldContextMenu.targetClick(
                point,
                controlRoot,
                /*spellcheckHighlighterInstantiator*/ null,
                /*mousePosition*/ null,
            );
        }
    }

    Keys.onPressed: event => {
        // trigger if context menu button is pressed
        Private.TextFieldContextMenu.targetKeyPressed(event, controlRoot)

        // Disable undo action for security reasons
        // See QTBUG-103934
        if ((echoMode === TextInput.PasswordEchoOnEdit || echoMode === TextInput.Password) && event.matches(StandardKey.Undo)) {
            event.accepted = true
        }
    }

    onPressAndHold: event => {
        if (!Kirigami.Settings.tabletMode) {
            return;
        }
        forceActiveFocus();
        cursorPosition = positionAt(event.x, event.y);
        selectWord();
    }

    Private.MobileCursor {
        target: controlRoot
        selectionStartHandle: true
        readonly property rect rect: {
            // selectionStart is actually the "physical" start of selection,
            // not a "logical" one, so we have to find out the real logical
            // end of selection ourselves.
            const positionEnd = controlRoot.cursorPosition === controlRoot.selectionStart ? controlRoot.selectionEnd : controlRoot.selectionStart;
            return controlRoot.positionToRectangle(positionEnd);
        }
        x: rect.x + controlRoot.leftPadding
        y: rect.y + controlRoot.topPadding
    }

    Text {
        id: placeholder
        x: controlRoot.leftPadding
        y: controlRoot.topPadding
        width: controlRoot.width - controlRoot.leftPadding - controlRoot.rightPadding
        height: controlRoot.height - controlRoot.topPadding - controlRoot.bottomPadding

        text: controlRoot.placeholderText
        font: controlRoot.font
        color: controlRoot.placeholderTextColor
        LayoutMirroring.enabled: false
        horizontalAlignment: controlRoot.effectiveHorizontalAlignment
        verticalAlignment: controlRoot.verticalAlignment
        visible: !controlRoot.length && !controlRoot.preeditText && (!controlRoot.activeFocus || controlRoot.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
    }

    background: StylePrivate.StyleItem {
        control: controlRoot
        elementType: "edit"

        sunken: true
        hasFocus: controlRoot.activeFocus
        hover: controlRoot.hovered
    }
}
