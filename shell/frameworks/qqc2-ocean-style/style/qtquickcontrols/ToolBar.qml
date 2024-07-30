/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import org.kde.ocean.impl as Impl

T.ToolBar {
    id: control

    readonly property bool __isHeader: control.position === T.ToolBar.Header
    readonly property bool __isFooter: control.position === T.ToolBar.Footer

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: LingmoUI.Units.smallSpacing

    padding: LingmoUI.Units.smallSpacing

    // Add space for the separator above the footer
    topPadding: __isFooter && background && background.hasOwnProperty("separator") ?
        background.separator.height + verticalPadding : verticalPadding
    // Add space for the separator below the header
    bottomPadding: __isHeader && background && background.hasOwnProperty("separator") ?
        background.separator.height + verticalPadding : verticalPadding

    LingmoUI.Theme.colorSet: __isHeader ? LingmoUI.Theme.Header : LingmoUI.Theme.Window
    LingmoUI.Theme.inherit: false

    background: Rectangle {
        // Enough height for Buttons/ComboBoxes/TextFields with smallSpacing padding on top and bottom
        implicitHeight: Impl.Units.mediumControlHeight + (LingmoUI.Units.smallSpacing * 2) + (separator.visible ? separator.height : 0)
        color: LingmoUI.Theme.backgroundColor
        property Item separator: LingmoUI.Separator {
            parent: background
            visible: control.__isHeader || control.__isFooter
            width: parent.width
            y: control.__isFooter ? 0 : parent.height - height
        }
    }
}
