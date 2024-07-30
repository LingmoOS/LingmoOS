/*
 * This file is part of KQuickCharts
 * SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.kde.quickcharts as Charts
import org.kde.quickcharts.controls

/**
 * A delegate that can be used as part of a Legend.
 */
Control {
    id: control

    property string name
    property string shortName
    property color color
    property string value

    property real maximumValueWidth

    property Component indicator: Rectangle {
        implicitWidth: Theme.cornerRadius
        color: control.color
    }

    property bool highlighted: false

    readonly property real minimumWidth: contentItem.minimumWidth
    readonly property real preferredWidth: contentItem.preferredWidth

    implicitHeight: Math.max(implicitContentHeight, implicitBackgroundHeight) + topPadding + bottomPadding

    // Note: Do not use implicitContentWidth here as it indirectly depends on the item width and will lead to a
    // nasty infinite layout loop. Instead use something more stable that doesn't change depending on the item
    // width.
    implicitWidth: Math.max(contentItem.preferredWidth, implicitBackgroundWidth) + leftPadding + rightPadding

    hoverEnabled: true

    leftPadding: 0
    rightPadding: Theme.cornerRadius
    topPadding: 0
    bottomPadding: 0

    spacing: Theme.smallSpacing

    contentItem: RowLayout {
        property real actualValueWidth: control.maximumValueWidth > 0 ? control.maximumValueWidth : value.implicitWidth
        property real minimumValueWidth: control.width - indicator.width - control.spacing

        property real minimumWidth: indicator.width + actualValueWidth + control.spacing
        property real preferredWidth: Math.ceil(indicator.width) + Math.ceil(name.implicitWidth) + Math.ceil(actualValueWidth) + Math.ceil(control.spacing * 2) + control.leftPadding + control.rightPadding

        spacing: control.spacing

        Loader {
            id: indicator

            Layout.preferredWidth: item ? item.implicitWidth : 0
            Layout.fillHeight: true

            sourceComponent: control.indicator
        }

        Label {
            id: name

            Layout.fillWidth: true
            Layout.fillHeight: true

            text: control.name + (control.shortName.length > 0 ? "\x9C" + control.shortName : "")
            elide: Text.ElideRight
            font: control.font
            verticalAlignment: Qt.AlignVCenter

            // In some cases we can trigger a behaviour where the full text is
            // displayed while we have a size of 0. In that case, explicitly
            // hide it. This uses opacity to ensure the item is still begin laid
            // out.
            opacity: width > 0 ? 1 : 0
        }

        Label {
            id: value

            Layout.fillHeight: true
            Layout.fillWidth: true

            Layout.minimumWidth: Math.min(parent.actualValueWidth, parent.minimumValueWidth)

            text: control.value;
            elide: Text.ElideRight
            font: name.font

            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignRight
        }
    }

    background: Rectangle {
        color: Qt.alpha(control.color, 0.25)
        border.color: control.color
        border.width: 1
        radius: Theme.cornerRadius
        visible: control.highlighted
    }

    ToolTip.visible: control.hovered && (name.truncated || value.truncated)
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.text: "%1: %2".arg(control.name).arg(control.value)
}

