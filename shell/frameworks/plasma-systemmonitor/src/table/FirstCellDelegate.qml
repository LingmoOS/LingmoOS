/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQml.Models

import org.kde.kirigami as Kirigami

BaseCellDelegate {
    id: delegate

    property string iconName

    property real iconSize: Kirigami.Units.iconSizes.smallMedium

    property bool treeDecorationVisible: false
    Binding {
        delegate.isTreeNode: treeDecorationVisible
    }
    onIsTreeNodeChanged: delegate.isTreeNode = Qt.binding(() => treeDecorationVisible)

    leftMargin: Kirigami.Units.largeSpacing

    contentItem: RowLayout {
        id: row

        // This is so the TreeViewDelegate from the style can detect when to show a tooltip.
        // Since our label is one level under, it doesn't have access to `truncated` normally.
        property alias truncated: label.truncated

        Kirigami.Icon {
            Layout.preferredWidth: delegate.iconSize
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignVCenter
            source: delegate.iconName
            fallback: ""
            animated: false
        }
        Label {
            id: label
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: delegate.text
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    hoverEnabled: true
}
