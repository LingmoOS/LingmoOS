/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as PC3
import org.kde.lingmo.extras as LingmoExtras
import org.kde.milou as Milou

import org.kde.lingmo.welcome

Item {
    id: root

    property string searchText: ""

    implicitWidth: layout.implicitWidth + layout.anchors.margins * 2
    implicitHeight: layout.implicitHeight + layout.anchors.margins * 2

    KSvg.FrameSvgItem {
        anchors.fill: background

        anchors.topMargin: -margins.top
        anchors.leftMargin: -margins.left
        anchors.rightMargin: -margins.right
        anchors.bottomMargin: -margins.bottom

        imagePath: "dialogs/background"
        prefix: "shadow"
    }

    KSvg.FrameSvgItem {
        id: background
        anchors.fill: parent

        imagePath: "dialogs/background"
        enabledBorders: KSvg.FrameSvg.BottomBorder | KSvg.FrameSvg.LeftBorder | KSvg.FrameSvg.RightBorder
    }

    ColumnLayout {
        id: layout
        anchors.fill: root
        anchors.margins: LingmoUI.Units.smallSpacing

        spacing: 0

        RowLayout {
            LingmoUI.Icon {
                Layout.margins: LingmoUI.Units.smallSpacing

                implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                implicitWidth: LingmoUI.Units.iconSizes.smallMedium

                source: "configure"
            }

            // Wrapped so disabling the search field does not affect the icons
            // and we can position search/clear icons with anchors
            Item {
                Layout.fillWidth: true

                implicitWidth: LingmoUI.Units.gridUnit * 25
                implicitHeight: field.implicitHeight

                LingmoExtras.ActionTextField {
                    id: field
                    anchors.fill: parent

                    enabled: false
                    opacity: 1

                    color: LingmoCore.Theme.textColor
                    placeholderTextColor: LingmoCore.Theme.disabledTextColor

                    placeholderText: i18nd("liblingmo6", "Search…")
                    text: root.searchText

                    cursorVisible: true

                    Timer {
                        id: cursorVisibleTimer

                        running: true
                        repeat: true
                        interval: Qt.styleHints.cursorFlashTime / 2
                        onTriggered: field.cursorVisible = !field.cursorVisible
                    }

                    background: KSvg.FrameSvgItem {
                        implicitWidth: LingmoUI.Units.gridUnit * 8 + margins.left + margins.right
                        implicitHeight: LingmoUI.Units.gridUnit + margins.top + margins.bottom
                        imagePath: "widgets/lineedit"
                        prefix: "base"

                        KSvg.FrameSvgItem {
                            anchors.fill: parent
                            anchors.leftMargin: -margins.left
                            anchors.topMargin: -margins.top
                            anchors.rightMargin: -margins.right
                            anchors.bottomMargin: -margins.bottom

                            imagePath: "widgets/lineedit"
                            prefix: "focus"
                        }
                    }

                    readonly property bool hasMargins: field.background.hasOwnProperty("margins")

                    leftPadding: searchIcon.width + searchIcon.anchors.leftMargin + (hasMargins ? field.background.margins.left : 0)
                    rightPadding: clearIcon.width + clearIcon.anchors.rightMargin + (hasMargins ? field.background.margins.right : 0)
                }

                LingmoUI.Icon {
                    id: searchIcon
                    anchors.left: field.left
                    anchors.leftMargin: LingmoUI.Units.smallSpacing * 2
                    anchors.verticalCenter: field.verticalCenter
                    anchors.verticalCenterOffset: Math.round((field.topPadding - field.bottomPadding) / 2)

                    implicitHeight: LingmoUI.Units.iconSizes.sizeForLabels
                    implicitWidth: LingmoUI.Units.iconSizes.sizeForLabels

                    source: "search"
                }

                LingmoUI.Icon {
                    id: clearIcon

                    anchors.right: field.right
                    anchors.rightMargin: LingmoUI.Units.smallSpacing * 2
                    anchors.verticalCenter: field.verticalCenter
                    anchors.verticalCenterOffset: Math.round((field.topPadding - field.bottomPadding) / 2)

                    readonly property bool isClear: field.text.length > 0

                    implicitHeight: isClear ? LingmoUI.Units.iconSizes.sizeForLabels : LingmoUI.Units.iconSizes.smallMedium
                    implicitWidth: isClear ? LingmoUI.Units.iconSizes.sizeForLabels : LingmoUI.Units.iconSizes.smallMedium

                    // "Expand" icon looks slightly different in KRunner, likely implemented on
                    // top of the search field ...there are limits to what we should copy
                    source: isClear ? "edit-clear-locationbar-rtl" : "expand"
                }
            }

            LingmoUI.Icon {
                Layout.margins: LingmoUI.Units.smallSpacing

                implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                implicitWidth: LingmoUI.Units.iconSizes.smallMedium

                source: "question"
            }

            LingmoUI.Icon {
                Layout.margins: LingmoUI.Units.smallSpacing

                implicitHeight: LingmoUI.Units.iconSizes.smallMedium
                implicitWidth: LingmoUI.Units.iconSizes.smallMedium

                source: "window-pin"
            }
        }

        PC3.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: LingmoUI.Units.smallSpacing

            visible: results.count > 0

            // HACK: Prevent tab focusing of children
            onFocusChanged: root.focus = true

            Milou.ResultsView {
                id: results

                queryString: root.searchText
            }
        }
    }

    // Eat mouse events (hover, click, scroll)
    MouseArea {
        anchors.fill: parent

        acceptedButtons: Qt.AllButtons
        hoverEnabled: true

        onWheel: wheel => wheel.accepted = true
    }
}
