/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.lingmoui as LingmoUI

/**
 * Base delegate for KControlmodules based on Grid views of thumbnails
 * Use the onClicked signal handler for managing the main action when
 * the user clicks on the tile, modified from the original GridDelegate
 * from the KCM module
 * @inherits QtQuick.Templates.ItemDelegate
 */
T.ItemDelegate {
    id: delegate

    /**
     * toolTip: string
     * string for a tooltip for the whole delegate
     */
    property string toolTip

    /**
     * tile: Item
     * the item actually implementing the tile: the visualization is up to the implementation
     */
    property alias tile: contentArea.data

    /**
     * thumbnailAvailable: bool
     * Set it to true when a tile is actually available: when false,
     * a default icon will be shown instead of the actual tile.
     */
    property bool thumbnailAvailable: false

    /**
     * actions: list<Action>
     * A list of extra actions for the thumbnails. They will be shown as
     * icons on the bottom-right corner of the tile on mouse over
     */
    property list<QtObject> actions

    /**
     * actionsAnchors: anchors
     * The anchors of the actions listing
     */
    property alias actionsAnchors: actionsScope.anchors

    /**
     * thumbnailArea: Item
     * The item that will contain the thumbnail within the delegate
     */
    property Item thumbnailArea : contentArea

    width: GridView.view.cellWidth
    height: GridView.view.cellHeight
    hoverEnabled: true

    LingmoUI.ShadowedRectangle {
        id: tile
        anchors.centerIn: parent
        width: LingmoUI.Settings.isMobile
            ? delegate.width - LingmoUI.Units.gridUnit
            : Math.min(delegate.GridView.view.implicitCellWidth, delegate.width - LingmoUI.Units.gridUnit)
        height: Math.min(delegate.GridView.view.implicitCellHeight, delegate.height - LingmoUI.Units.gridUnit)
        radius: LingmoUI.Units.smallSpacing
        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.View

        shadow.xOffset: 0
        shadow.yOffset: 2
        shadow.size: 10
        shadow.color: Qt.rgba(0, 0, 0, 0.3)

        color: {
            // Otherwise the first item is focused, BUG: 417843
            // We should rethink this when fixing the keyboard navigation
            /*if (delegate.GridView.isCurrentItem) {
                return LingmoUI.Theme.highlightColor;
            } else */ if (parent.hovered) {
                // Match appearance of hovered list items
                return Qt.alpha(LingmoUI.Theme.highlightColor, 0.5);

            } else {
                return LingmoUI.Theme.backgroundColor;
            }
        }
        Behavior on color {
            ColorAnimation {
                duration: LingmoUI.Units.longDuration
                easing.type: Easing.OutQuad
            }
        }

        Rectangle {
            id: contentArea
            radius: LingmoUI.Units.smallSpacing/2
            anchors {
                fill: parent
                margins: LingmoUI.Units.smallSpacing
            }

            color: LingmoUI.Theme.backgroundColor
        }

        LingmoUI.Icon {
            parent: thumbnailArea
            visible: !delegate.thumbnailAvailable
            anchors.centerIn: parent
            width: LingmoUI.Units.iconSizes.large
            height: width
            source: delegate.text === i18nd("knewstuff6", "None") ? "edit-none" : "view-preview"
        }

        Rectangle {
            anchors.fill: contentArea
            visible: actionsColumn.children.length > 0
            opacity: LingmoUI.Settings.isMobile || delegate.hovered || (actionsScope.focus) ? 1 : 0
            radius: LingmoUI.Units.smallSpacing
            color: LingmoUI.Settings.isMobile ? "transparent" : Qt.rgba(1, 1, 1, 0.2)

            Behavior on opacity {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.OutQuad
                }
            }

            FocusScope {
                id: actionsScope

                anchors {
                    right: parent.right
                    rightMargin: LingmoUI.Units.smallSpacing
                    top: parent.top
                    topMargin: LingmoUI.Units.smallSpacing
                }
                width: actionsColumn.width
                height: actionsColumn.height

                ColumnLayout {
                    id: actionsColumn

                    Repeater {
                        model: delegate.actions
                        delegate: QQC2.Button {
                            icon.name: modelData.iconName
                            text: modelData.text
                            activeFocusOnTab: focus || delegate.focus
                            onClicked: modelData.trigger()
                            enabled: modelData.enabled
                            visible: modelData.visible
                            //NOTE: there aren't any global settings where to take "official" tooltip timeouts
                            QQC2.ToolTip.delay: 1000
                            QQC2.ToolTip.timeout: 5000
                            QQC2.ToolTip.visible: (LingmoUI.Settings.isMobile ? pressed : hovered) && modelData.tooltip.length > 0
                            QQC2.ToolTip.text: modelData.tooltip
                        }
                    }
                }
            }
        }
    }

    QQC2.ToolTip.delay: 1000
    QQC2.ToolTip.timeout: 5000
    QQC2.ToolTip.visible: hovered && delegate.toolTip.length > 0
    QQC2.ToolTip.text: toolTip
}
