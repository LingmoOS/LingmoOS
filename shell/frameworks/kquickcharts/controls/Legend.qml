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
 * A pre-made legend control that displays a legend for charts.
 */
Control {
    id: control

    /**
     * The chart to display the legend for.
     */
    property Charts.Chart chart
    /**
     * The delegate to use to display legend information.
     *
     * \sa Legend::delegate
     */
    property alias delegate: legendRepeater.delegate
    /**
     *
     */
    property alias model: legendRepeater.model

    property alias horizontalSpacing: legend.horizontalSpacing
    property alias verticalSpacing: legend.verticalSpacing

    property real maximumDelegateWidth: Theme.gridUnit * 10

    property var formatValue: function(input, index) { return input }
    property var maximumValueWidth: function(input, index) { return -1 }

    property alias preferredWidth: legend.preferredWidth

    property string nameRole: "name"
    property string shortNameRole: "shortName"
    property string colorRole: "color"
    property string valueRole: "value"

    property bool highlightEnabled: false
    property int highlightedIndex: -1

    default property alias _children: legend.children

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    implicitWidth: Math.max(implicitContentWidth, implicitBackgroundWidth) + leftPadding + rightPadding
    implicitHeight: Math.max(implicitContentHeight, implicitBackgroundHeight) + topPadding + bottomPadding

    contentItem: Flickable {
        anchors.fill: parent

        contentHeight: legend.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        implicitHeight: legend.implicitHeight
        implicitWidth: legend.implicitWidth

        // Limit maximum flick velocity to ensure we can scroll one line per
        // mouse wheel "tick" when the legend's height is very constrained.
        maximumFlickVelocity: Theme.gridUnit * 50
        LegendLayout {
            id: legend

            width: parent.width

            Repeater {
                id: legendRepeater

                model: LegendModel { chart: control.chart }

                delegate: LegendDelegate {
                    property var itemData: typeof modelData !== "undefined" ? modelData : model

                    name: itemData[control.nameRole] ?? ""
                    shortName: itemData[control.shortNameRole] ?? ""
                    color: itemData[control.colorRole] ?? "white"
                    value: control.formatValue(itemData[control.valueRole] ?? "", index)

                    highlighted: control.highlightEnabled && hovered

                    maximumValueWidth: {
                        var result = control.maximumValueWidth(model.value, index)
                        if (result > 0) {
                            return result
                        }

                        return -1
                    }

                    LegendLayout.minimumWidth: minimumWidth
                    LegendLayout.preferredWidth: preferredWidth
                    LegendLayout.maximumWidth: Math.max(control.maximumDelegateWidth, preferredWidth)

                    onHoveredChanged: {
                        if (control.highlightEnabled) {
                            if (hovered) {
                                control.highlightedIndex = index
                            } else if (control.highlightedIndex == index) {
                                control.highlightedIndex = -1
                            }
                        }
                    }
                }
            }

            horizontalSpacing: Theme.smallSpacing
            verticalSpacing: Theme.smallSpacing
        }

        children: [
            Item {
                width: parent.width;
                height: 1;
                visible: parent.contentY > 0

                ToolButton {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        top: parent.top
                    }

                    width: Theme.smallIconSize
                    height: Theme.smallIconSize

                    icon.name: "arrow-up-symbolic"
                    icon.width: Theme.smallIconSize
                    icon.height: Theme.smallIconSize
                    enabled: false
                }
            },
            Item {
                y: parent.height - height
                width: parent.width;
                height: 1;
                visible: parent.contentY + parent.height < legend.height

                ToolButton {
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        bottom: parent.bottom
                    }

                    width: Theme.smallIconSize
                    height: Theme.smallIconSize

                    icon.name: "arrow-down-symbolic"
                    icon.width: Theme.smallIconSize
                    icon.height: Theme.smallIconSize
                    enabled: false
                }
            }
        ]
    }
}
