/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents3

RowLayout {
    id: toolButtonsLayout
    visible: menuItem.ListView.isCurrentItem

    // https://bugreports.qt.io/browse/QTBUG-108821
    readonly property bool hovered: actionToolButton.hovered || barcodeToolButton.hovered || editToolButton.hovered || deleteToolButton.hovered

    LingmoComponents3.ToolButton {
        id: actionToolButton
        // TODO: only show for items supporting actions?
        icon.name: "system-run"

        display: LingmoComponents3.AbstractButton.IconOnly
        text: i18n("Invoke action")

        onClicked: menuItem.triggerAction(UuidRole)

        LingmoComponents3.ToolTip {
            text: parent.text
        }
        KeyNavigation.right: barcodeToolButton
    }
    LingmoComponents3.ToolButton {
        id: barcodeToolButton
        icon.name: "view-barcode-qr"
        visible: supportsBarcodes

        display: LingmoComponents3.AbstractButton.IconOnly
        text: i18n("Show QR code")

        onClicked: menuItem.barcode(DisplayRole)

        LingmoComponents3.ToolTip {
            text: parent.text
        }
        KeyNavigation.right: editToolButton
    }
    LingmoComponents3.ToolButton {
        id: editToolButton
        icon.name: "document-edit"
        enabled: !clipboardSource.editing
        visible: TypeRole === 0

        display: LingmoComponents3.AbstractButton.IconOnly
        text: i18n("Edit contents")

        onClicked: menuItem.edit(UuidRole)

        LingmoComponents3.ToolTip {
            text: parent.text
        }
        KeyNavigation.right: deleteToolButton
    }
    LingmoComponents3.ToolButton {
        id: deleteToolButton
        icon.name: "edit-delete"

        display: LingmoComponents3.AbstractButton.IconOnly
        text: i18n("Remove from history")

        onClicked: menuItem.remove(UuidRole)

        LingmoComponents3.ToolTip {
            text: parent.text
        }
    }
}
