/*
 * SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.ksvg as KSvg
import org.kde.lingmoui as LingmoUI
import "../" as LingmoComponents

RowLayout {
    id: root

    required property string labelText
    required property T.Button button

    readonly property bool usingFocusBackground: !button.flat && buttonSvg.hasElement("hint-focus-highlighted-background") && button.visualFocus && !(button.pressed || button.checked)
    readonly property int defaultIconSize: button.flat ? LingmoUI.Units.iconSizes.smallMedium : LingmoUI.Units.iconSizes.small

    // Can't rely on the transient Item::visible property
    readonly property bool iconVisible: icon.source.toString() !== "" && button.display !== T.Button.TextOnly
    readonly property bool labelVisible: labelText !== "" && root.button.display !== T.Button.IconOnly

    spacing: button.spacing

    GridLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        columns: root.button.display == T.Button.TextBesideIcon ? 2 : 1
        rowSpacing: root.button.spacing
        columnSpacing: rowSpacing

        LingmoUI.Icon {
            id: icon

            Layout.alignment: Qt.AlignCenter

            Layout.fillWidth: root.button.display !== T.Button.TextBesideIcon || root.labelText === ""
            Layout.fillHeight: true

            Layout.minimumWidth: Math.min(root.width, root.height, implicitWidth)
            Layout.minimumHeight: Math.min(root.width, root.height, implicitHeight)

            Layout.maximumWidth: root.button.icon.width > 0 ? root.button.icon.width : Number.POSITIVE_INFINITY
            Layout.maximumHeight: root.button.icon.height > 0 ? root.button.icon.height : Number.POSITIVE_INFINITY

            implicitWidth: root.button.icon.width > 0 ? root.button.icon.width : root.defaultIconSize
            implicitHeight: root.button.icon.height > 0 ? root.button.icon.height : root.defaultIconSize
            visible: root.iconVisible
            source: root.button.icon.name !== "" ? root.button.icon.name : root.button.icon.source
            color: root.button.icon.color
            selected: root.usingFocusBackground
        }
        LingmoComponents.Label {
            id: label
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.labelVisible
            text: root.labelText
            font: root.button.font
            color: root.usingFocusBackground ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
            horizontalAlignment: root.button.display !== T.Button.TextUnderIcon && root.iconVisible ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
    KSvg.SvgItem {
        visible: root.button.Accessible.role === Accessible.ButtonMenu && root.labelVisible
        Layout.preferredWidth: LingmoUI.Units.iconSizes.small
        Layout.preferredHeight: Layout.preferredWidth
        Layout.alignment: Qt.AlignCenter
        imagePath: "widgets/arrows"
        elementId: "down-arrow"
    }
    KSvg.Svg {
        id: buttonSvg
        imagePath: "widgets/button"
    }
}
