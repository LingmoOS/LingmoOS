/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T
import org.kde.ksvg as KSvg
//NOTE: importing PlasmaCore is necessary in order to make KSvg load the current Plasma Theme
import org.kde.plasma.core as PlasmaCore
import "private" as Private
import org.kde.kirigami as Kirigami
import "mobiletextselection" as MobileTextSelection

T.TextArea {
    id: control

    implicitWidth: Math.max(contentWidth + leftPadding + rightPadding,
                            background ? background.implicitWidth : 0,
                            placeholder.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                             background ? background.implicitHeight : 0,
                             placeholder.implicitHeight + topPadding + bottomPadding)

    padding: 6

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false
    color: Kirigami.Theme.textColor
    selectionColor: Kirigami.Theme.highlightColor
    selectedTextColor: Kirigami.Theme.highlightedTextColor
    opacity: control.enabled ? 1 : 0.6
    verticalAlignment: TextEdit.AlignTop
    hoverEnabled: !Kirigami.Settings.tabletMode || !Kirigami.Settings.hasTransientTouchInput

    selectByMouse: hoverEnabled

    cursorDelegate: !hoverEnabled ? mobileCursor : null
    Component {
        id: mobileCursor
        MobileTextSelection.MobileCursor {
            target: control
        }
    }

    onTextChanged: MobileTextSelection.MobileTextActionsToolBar.shouldBeVisible = false;
    onPressed: MobileTextSelection.MobileTextActionsToolBar.shouldBeVisible = true;

    onPressAndHold: {
        if (hoverEnabled) {
            return;
        }
        forceActiveFocus();
        cursorPosition = positionAt(event.x, event.y);
        selectWord();
    }

    MobileTextSelection.MobileCursor {
        target: control
        selectionStartHandle: true
        property var rect: target.positionToRectangle(target.selectionStart)
        x: rect.x
        y: rect.y
    }

    onFocusChanged: {
        if (focus) {
            MobileTextSelection.MobileTextActionsToolBar.controlRoot = control;
        }
    }

    Label {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: control.font
        color: Kirigami.Theme.textColor
        opacity: 0.5
        enabled: false
        horizontalAlignment: control.horizontalAlignment
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
    }

    background: Item {
        Private.TextFieldFocus {
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "hidden")
            anchors.fill: parent
        }
        KSvg.FrameSvgItem {
            id: base
            anchors.fill: parent
            imagePath: "widgets/lineedit"
            prefix: "base"
        }
    }
}
