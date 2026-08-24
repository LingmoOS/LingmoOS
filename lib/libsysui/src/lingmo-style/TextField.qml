/*
 * Copyright 2017 Marco Martin <mart@kde.org>
 * Copyright 2017 The Qt Company Ltd.
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */


import QtQuick 2.6
import QtQuick.Window 2.1
import QtQuick.Controls 2.3 as Controls
import QtQuick.Templates 2.3 as T
import LingmoUI 1.0 as LingmoUI

T.TextField {
    id: control

    implicitWidth: Math.max(200,
                            placeholderText ? placeholder.implicitWidth + leftPadding + rightPadding : 0)
                            || contentWidth + leftPadding + rightPadding + LingmoUI.Units.extendBorderWidth
    implicitHeight: Math.max(contentHeight + topPadding + bottomPadding,
                             background ? background.implicitHeight : 0,
                             placeholder.implicitHeight + topPadding + bottomPadding + LingmoUI.Units.extendBorderWidth)

    // padding: 6
    leftPadding: LingmoUI.Units.smallSpacing + LingmoUI.Units.extendBorderWidth
    rightPadding: LingmoUI.Units.smallSpacing + LingmoUI.Units.extendBorderWidth

    //Text.NativeRendering is broken on non integer pixel ratios
    // renderType: Window.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering
    renderType: LingmoUI.Theme.renderType

    color: control.enabled ? LingmoUI.Theme.textColor : LingmoUI.Theme.disabledTextColor
    selectionColor: LingmoUI.Theme.highlightColor
    selectedTextColor: LingmoUI.Theme.highlightedTextColor
    selectByMouse: true

    horizontalAlignment: Text.AlignLeft
    verticalAlignment: TextInput.AlignVCenter

    opacity: control.enabled ? 1.0 : 0.5

 	// cursorDelegate: CursorDelegate { }

    Controls.Label {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: control.font
        color: LingmoUI.Theme.textColor
        opacity: 0.4
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
        wrapMode: Text.NoWrap
	}

    background: Rectangle {
        implicitWidth: (LingmoUI.Units.iconSizes.medium * 3) + LingmoUI.Units.smallSpacing + LingmoUI.Units.extendBorderWidth
        implicitHeight: LingmoUI.Units.iconSizes.medium + LingmoUI.Units.smallSpacing + LingmoUI.Units.extendBorderWidth
        // color: control.activeFocus ? Qt.lighter(LingmoUI.Theme.backgroundColor, 1.4) : LingmoUI.Theme.backgroundColor
        color: LingmoUI.Theme.alternateBackgroundColor
        radius: LingmoUI.Theme.smallRadius

        border.width: 1
        border.color: control.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.alternateBackgroundColor

        // Rectangle {
        //     id: _border
        //     anchors.fill: parent
        //     color: "transparent"
        //     border.color: control.activeFocus ? Qt.rgba(LingmoUI.Theme.highlightColor.r,
        //                                                 LingmoUI.Theme.highlightColor.g,
        //                                                 LingmoUI.Theme.highlightColor.b, 0.2) : "transparent"
        //     border.width: LingmoUI.Units.extendBorderWidth
        //     radius: LingmoUI.Theme.smallRadius + LingmoUI.Units.extendBorderWidth

        //     Behavior on border.color {
        //         ColorAnimation {
        //             duration: 50
        //         }
        //     }
        // }

        // Rectangle {
        //     anchors.fill: parent
        //     anchors.margins: LingmoUI.Units.extendBorderWidth
        //     radius: LingmoUI.Theme.smallRadius
        //     color: LingmoUI.Theme.backgroundColor
        //     border.color: control.activeFocus ? LingmoUI.Theme.highlightColor : Qt.tint(LingmoUI.Theme.textColor, Qt.rgba(LingmoUI.Theme.backgroundColor.r, LingmoUI.Theme.backgroundColor.g, LingmoUI.Theme.backgroundColor.b, 0.7))
        //     border.width: 1
        // }
    }
}
