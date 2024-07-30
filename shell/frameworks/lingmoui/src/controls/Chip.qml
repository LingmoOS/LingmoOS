// SPDX-FileCopyrightText: 2022 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

import "templates" as KT
import "private" as P

/**
 * @brief A compact element that represents an attribute, action, or filter.
 *
 * Should be used in a group of multiple elements. e.g when displaying tags in a image viewer.
 *
 * Example usage:
 *  * @code
 * import org.kde.lingmoui as LingmoUI
 *
 * Flow {
 *     Repeater {
 *         model: chipsModel
 *
 *         LingmoUI.Chip {
 *             text: model.text
 *             icon.name: "tag-symbolic"
 *             closable: model.closable
 *             onClicked: {
 *                 [...]
 *             }
 *             onRemoved: {
 *                 [...]
 *             }
 *         }
 *     }
 * }
 * @endcode
 *
 * @since 2.19
 */
KT.Chip {
    id: chip

    implicitWidth: layout.implicitWidth
    implicitHeight: toolButton.implicitHeight

    checkable: !closable

    /**
     * @brief This property holds the label item; used for accessing the usual Text properties.
     * @property QtQuick.Controls.Label labelItem
     */
    property alias labelItem: label

    contentItem: RowLayout {
        id: layout
        spacing: 0

        LingmoUI.Icon {
            id: icon
            visible: icon.valid
            Layout.preferredWidth: LingmoUI.Units.iconSizes.small
            Layout.preferredHeight: LingmoUI.Units.iconSizes.small
            Layout.leftMargin: LingmoUI.Units.smallSpacing
            color: chip.icon.color
            isMask: chip.iconMask
            source: chip.icon.name || chip.icon.source
        }
        QQC2.Label {
            id: label
            Layout.fillWidth: true
            Layout.minimumWidth: LingmoUI.Units.gridUnit * 1.5
            Layout.leftMargin: icon.visible ? LingmoUI.Units.smallSpacing : LingmoUI.Units.largeSpacing
            Layout.rightMargin: chip.closable ? LingmoUI.Units.smallSpacing : LingmoUI.Units.largeSpacing
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: chip.text
            color: LingmoUI.Theme.textColor
            elide: Text.ElideRight
        }
        QQC2.ToolButton {
            id: toolButton
            visible: chip.closable
            text: qsTr("Remove Tag")
            icon.name: "edit-delete-remove"
            icon.width: LingmoUI.Units.iconSizes.sizeForLabels
            icon.height: LingmoUI.Units.iconSizes.sizeForLabels
            display: QQC2.AbstractButton.IconOnly
            onClicked: chip.removed()
        }
    }

    background: P.DefaultChipBackground {}
}
