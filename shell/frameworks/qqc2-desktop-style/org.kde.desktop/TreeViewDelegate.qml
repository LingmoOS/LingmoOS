/*
    SPDX-FileCopyrightText: Copyright (C) 2022 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>
    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.qqc2desktopstyle.private as StylePrivate
import org.kde.desktop.private as Private

T.TreeViewDelegate {
    id: controlRoot

    implicitWidth: leftMargin + __contentIndent + implicitContentWidth  + rightMargin + (mirrored ? leftPadding : rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight, implicitContentHeight, implicitIndicatorHeight) + topPadding + bottomPadding

    // This is the default size of the indicator when using Ocean aka pixelMetric("treeviewindentation")
    indentation: indicator ? indicator.width : 20

    leftMargin: LingmoUI.Units.smallSpacing
    rightMargin: LingmoUI.Units.smallSpacing
    spacing:  LingmoUI.Units.smallSpacing

    leftPadding: !mirrored ? leftMargin + __contentIndent : 0
    rightPadding: mirrored ? leftMargin + __contentIndent : 0
    verticalPadding: LingmoUI.Units.smallSpacing

    leftInset: TableView.view ? 0 : horizontalPadding / 2
    rightInset: TableView.view ? 0 : horizontalPadding / 2
    topInset: TableView.view ? 0 : Math.ceil(verticalPadding / 2)
    bottomInset: TableView.view ? 0 : Math.ceil(verticalPadding / 2)

    LingmoUI.Theme.colorSet: highlighted ? LingmoUI.Theme.Selection : LingmoUI.Theme.View
    LingmoUI.Theme.inherit: false

    highlighted: controlRoot.selected || controlRoot.current
               || ((controlRoot.treeView.selectionBehavior === TableView.SelectRows
               || controlRoot.treeView.selectionBehavior === TableView.SelectionDisabled)
               && controlRoot.row === controlRoot.treeView.currentRow)

    icon.width: LingmoUI.Units.iconSizes.smallMedium
    icon.height: LingmoUI.Units.iconSizes.smallMedium

    T.ToolTip.visible: (LingmoUI.Settings.tabletMode ? down : hovered) && (contentItem.truncated ?? false)
    T.ToolTip.text: controlRoot.model.display
    T.ToolTip.delay: LingmoUI.Units.toolTipDelay

    required property int row
    required property int column
    required property var model
    readonly property real __contentIndent: !isTreeNode ? 0 : (depth * indentation) + (indicator ? indicator.width + spacing : 0)

    // TableView does not provide us with a source QModelIndex, so we have to
    // reconstruct it ourselves, in an (unfortunately) non-observable way.
    property /*QModelIndex*/var modelIndex: expressionForModelIndex()

    function expressionForModelIndex(): /*QModelIndex*/var {
        // Note: this is not observable in case of model changes
        return treeView.index(row, column);
    }

    function refreshModelIndex(): void {
        modelIndex = Qt.binding(() => expressionForModelIndex());
    }

    Component.onCompleted: {
        refreshModelIndex();
    }

    TableView.onReused: {
        refreshModelIndex();
    }

    // The indicator is only visible when the item has children, so  this is only the closest branch indicator (+arrow) - the rest of the branch indicator lines are below
    indicator: StylePrivate.StyleItem {
        readonly property real __indicatorIndent: controlRoot.leftMargin + (controlRoot.depth * controlRoot.indentation)
        x: !controlRoot.mirrored ? __indicatorIndent : controlRoot.width - __indicatorIndent - width
        height: parent.height
        width: pixelMetric("treeviewindentation")
        hover: hover.hovered
        elementType: "itembranchindicator"
        on: controlRoot.expanded
        selected: controlRoot.highlighted || controlRoot.checked || (controlRoot.pressed && !controlRoot.checked)
        properties: {
            "isItem": true,
            "hasChildren": true,
            "hasSibling": controlRoot.treeView.model.rowCount(controlRoot.modelIndex.parent) > controlRoot.modelIndex.row + 1
        }
        HoverHandler {
            id: hover
        }
    }

    // The rest of the branch indicators, this is outside of the background so consumers can freely
    // modify it without losing it
    StylePrivate.ItemBranchIndicators {
        visible: controlRoot.isTreeNode
        height: parent.height
        x: controlRoot.mirrored ? controlRoot.width - controlRoot.leftMargin - width : controlRoot.leftMargin
        modelIndex: controlRoot.modelIndex
        selected: controlRoot.highlighted || controlRoot.checked || (controlRoot.pressed && !controlRoot.checked)
        rootIndex: controlRoot.treeView.rootIndex
    }

    background: Private.DefaultListItemBackground {
        LingmoUI.Theme.colorSet: LingmoUI.Theme.View
        LingmoUI.Theme.inherit: false
        // This is intentional and ensures the inset is not directly applied to
        // the background, allowing it to determine how to handle the inset.
        anchors.fill: parent
        control: controlRoot
    }

    contentItem: RowLayout {
        LayoutMirroring.enabled: controlRoot.mirrored
        spacing: controlRoot.spacing

        property alias truncated: textLabel.truncated

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignVCenter
            visible: controlRoot.icon.name !== "" || controlRoot.icon.source.toString() !== "" || controlRoot.model.decoration !== undefined
            source: controlRoot.icon.name !== "" ? controlRoot.icon.name : controlRoot.icon.source.toString() !== "" ? controlRoot.icon.source : controlRoot.model.decoration
            Layout.preferredHeight: controlRoot.icon.height
            Layout.preferredWidth: controlRoot.icon.width
            selected: controlRoot.highlighted || controlRoot.checked || (controlRoot.pressed && !controlRoot.checked)
        }
        Label {
            id: textLabel

            Layout.fillWidth: true
            Layout.fillHeight: true

            text: controlRoot.model.display
            font: controlRoot.font
            color: controlRoot.highlighted || controlRoot.checked || (controlRoot.pressed && !controlRoot.checked)
                ? LingmoUI.Theme.highlightedTextColor
                : (controlRoot.enabled ? LingmoUI.Theme.textColor : LingmoUI.Theme.disabledTextColor)
            elide: Text.ElideRight
            visible: text !== ""
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }
}
