/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import "private" as Private

/**
 * Base delegate for KControlmodules based on Grid views of thumbnails
 * Use the onClicked signal handler for managing the main action when
 * the user clicks on the thumbnail
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
     * subtitle: string
     * optional string for the text to show below the main label
     */
    property string subtitle

    /**
     * thumbnail: Item
     * the item actually implementing the thumbnail: the visualization is up to the implementation
     */
    property alias thumbnail: thumbnailArea.data

    /**
     * thumbnailAvailable: bool
     * Set it to true when a thumbnail is actually available: when false,
     * only an icon will be shown instead of the actual thumbnail
     * ("edit-none" if pluginName is "None", otherwise it uses "view-preview").
     */
    property bool thumbnailAvailable: false

    /**
     * actions: list<LingmoUI.Action>
     * A list of extra actions for the thumbnails. They will be shown as
     * icons on the bottom-right corner of the thumbnail on mouse over
     */
    property list<LingmoUI.Action> actions

    width: GridView.view.cellWidth
    height: GridView.view.cellHeight
    hoverEnabled: !LingmoUI.Settings.isMobile

    Accessible.description: {
        if (toolTip.length === 0) {
            return subtitle;
        } else if (subtitle.length === 0) {
            return toolTip;
        }
        return `${subtitle}; ${toolTip}`
    }

    Keys.onEnterPressed: event => thumbnail.trigger()
    Keys.onMenuPressed: event => thumbnail.trigger()
    Keys.onSpacePressed: event => thumbnail.trigger()

    LingmoUI.ShadowedRectangle {
        id: thumbnail
        anchors {
           centerIn: parent
           verticalCenterOffset: Math.ceil(-labelLayout.height / 2)
        }
        width: LingmoUI.Settings.isMobile ? delegate.width - LingmoUI.Units.gridUnit : Math.min(delegate.GridView.view.implicitCellWidth, delegate.width - LingmoUI.Units.gridUnit)
        height: LingmoUI.Settings.isMobile ? Math.round((delegate.width - LingmoUI.Units.gridUnit) / 1.6)
                                           : Math.min(delegate.GridView.view.implicitCellHeight - LingmoUI.Units.gridUnit * 3,
                                                      delegate.height - LingmoUI.Units.gridUnit)
        radius: LingmoUI.Units.cornerRadius
        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.View

        shadow.xOffset: 0
        shadow.yOffset: 2
        shadow.size: 10
        shadow.color: Qt.rgba(0, 0, 0, 0.3)

        color: {
            if (delegate.GridView.isCurrentItem) {
                if (delegate.enabled && delegate.GridView.view.neutralHighlight) {
                    return LingmoUI.Theme.neutralTextColor;
                }
                return LingmoUI.Theme.highlightColor;
            }
            if (delegate.enabled && delegate.hovered) {
                // Match appearance of hovered list items
                return Qt.alpha(LingmoUI.Theme.highlightColor, 0.5);
            }
            return LingmoUI.Theme.backgroundColor;
        }

        // The menu is only used for keyboard navigation, so no need to always load
        // it. This speeds up the compilation of GridDelegate.
        property Private.GridDelegateMenu menu

        function trigger() {
            if (!menu) {
                const component = Qt.createComponent("private/GridDelegateMenu.qml");
                menu = component.createObject(delegate);
                component.destroy();
            }

            menu.trigger();
        }

        Rectangle {
            id: thumbnailArea

            radius: Math.round(LingmoUI.Units.cornerRadius / 2)
            anchors {
                fill: parent
                margins: LingmoUI.Units.smallSpacing
            }

            color: LingmoUI.Theme.backgroundColor

            // "None/There's nothing here" indicator
            LingmoUI.Icon {
                visible: !delegate.thumbnailAvailable
                anchors.centerIn: parent
                width: LingmoUI.Units.iconSizes.large
                height: LingmoUI.Units.iconSizes.large
                source: typeof pluginName === "string" && pluginName === "None" ? "edit-none" : "view-preview"
            }

            RowLayout {
                anchors {
                    right: parent.right
                    rightMargin: LingmoUI.Units.smallSpacing
                    bottom: parent.bottom
                    bottomMargin: LingmoUI.Units.smallSpacing
                }
                spacing: LingmoUI.Units.smallSpacing

                // Always show above thumbnail content
                z: 9999

                visible: delegate.actions.length > 0 && (LingmoUI.Settings.isMobile || delegate.hovered || delegate.GridView.isCurrentItem)

                Repeater {
                    model: delegate.actions
                    delegate: QQC2.Button {
                        required property LingmoUI.Action modelData

                        icon.name: modelData.icon.name
                        text: modelData.text || modelData.tooltip
                        display: QQC2.AbstractButton.IconOnly

                        enabled: modelData.enabled
                        visible: modelData.visible

                        activeFocusOnTab: false

                        onClicked: modelData.trigger()

                        QQC2.ToolTip.visible: (LingmoUI.Settings.tabletMode ? pressed : hovered) && (QQC2.ToolTip.text !== "")
                        QQC2.ToolTip.delay: LingmoUI.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : LingmoUI.Units.toolTipDelay
                        QQC2.ToolTip.text: text
                    }
                }
            }
        }
    }

    ColumnLayout {
        id: labelLayout

        spacing: 0
        height: LingmoUI.Units.gridUnit * 2
        anchors {
            left: thumbnail.left
            right: thumbnail.right
            top: thumbnail.bottom
            topMargin: LingmoUI.Units.largeSpacing
        }

        QQC2.Label {
            id: title

            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
            text: delegate.text
            color: enabled ? LingmoUI.Theme.textColor : LingmoUI.Theme.disabledTextColor
            elide: Text.ElideRight
            font.bold: delegate.GridView.isCurrentItem
            textFormat: Text.PlainText
        }
        QQC2.Label {
            id: caption

            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            visible: delegate.subtitle.length > 0
            opacity: 0.6
            text: delegate.subtitle
            font.pointSize: LingmoUI.Theme.smallFont.pointSize
            font.bold: delegate.GridView.isCurrentItem
            elide: Text.ElideRight
            textFormat: Text.PlainText
        }

        Rectangle {
            Layout.preferredHeight: 1
            Layout.preferredWidth: Math.max(title.paintedWidth, caption.paintedWidth)
            Layout.maximumWidth: labelLayout.width // Otherwise labels can overflow
            Layout.alignment: Qt.AlignHCenter

            color: LingmoUI.Theme.highlightColor

            opacity: delegate.visualFocus ? 1 : 0
        }

        Item { Layout.fillWidth: true; Layout.fillHeight: true; }
    }

    QQC2.ToolTip.visible: (LingmoUI.Settings.tabletMode ? pressed : hovered) && (QQC2.ToolTip.text !== "")
    QQC2.ToolTip.delay: LingmoUI.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : LingmoUI.Units.toolTipDelay
    QQC2.ToolTip.text: {
        if (delegate.toolTip.length > 0) {
            return delegate.toolTip;
        }
        const parts = [];
        if (title.truncated) {
            parts.push(title.text);
        }
        if (caption.truncated) {
            parts.push(caption.text);
        }
        return parts.join("\n");
    }
}
