/*
 * SPDX-FileCopyrightText: 2024 Elysia <elysia@lingmo.org>
 *
 * SPDX-License-Identifier: GPL-3.0
 */

import QtQuick
import QtQuick.Templates as T
import LingmoUI
import "./"

T.Dialog {
    id: control

    property color resizeBorderColor: {
        if (window.active) {
            return LingmoTheme.dark ? Qt.rgba(
                                          51 / 255, 51 / 255, 51 / 255,
                                          1) : Qt.rgba(110 / 255, 110 / 255, 110 / 255, 1)
        }
        return LingmoTheme.dark ? Qt.rgba(
                                      61 / 255, 61 / 255, 61 / 255,
                                      1) : Qt.rgba(167 / 255, 167 / 255, 167 / 255, 1)
    }
    property int resizeBorderWidth: 1

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding,
                            implicitHeaderWidth,
                            implicitFooterWidth)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding
                             + (implicitHeaderHeight > 0 ? implicitHeaderHeight + spacing : 0)
                             + (implicitFooterHeight > 0 ? implicitFooterHeight + spacing : 0))

    padding: 24
    verticalPadding: 18

    background: Rectangle {
        color: LingmoTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
        border.width: control.resizeBorderWidth
        border.color: control.resizeBorderColor
        radius: LingmoUnits.windowRadius
    }

    header: LingmoText{
        id:text_title
        font: LingmoTextStyle.Title
        text:title
        topPadding: 20
        leftPadding: 20
        rightPadding: 20
        wrapMode: Text.WordWrap
    }

    footer: DialogButtonBox {
        visible: count > 0
    }

    T.Overlay.modal: Rectangle {
        radius: LingmoUnits.windowRadius
        color: LingmoTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
    }

    T.Overlay.modeless: Rectangle {
        radius: LingmoUnits.windowRadius
        color: LingmoTheme.dark ? Qt.rgba(32/255,32/255,32/255,1) : Qt.rgba(243/255,243/255,243/255,1)
    }
}
