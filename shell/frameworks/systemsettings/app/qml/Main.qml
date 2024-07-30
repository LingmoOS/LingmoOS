/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
   SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick

import org.kde.lingmoui as LingmoUI

Item {
    id: root

    // properties, signals and methods used by C++ backend

    readonly property real headerHeight: sideBar.headerHeight

    implicitHeight: sideBar.implicitHeight
    implicitWidth: sideBar.implicitWidth + separator.implicitWidth

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    SideBarItem {
        id: sideBar

        anchors.fill: parent
        anchors.rightMargin: separator.width
    }

    LingmoUI.Separator {
        id: separator
        z: 1
        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }
        LingmoUI.Theme.inherit: false
        LingmoUI.Theme.colorSet: LingmoUI.Theme.Header

        // These two rectangles on top of the separator are here because we're mimicking
        // the standard LingmoUI appearance that can't be inherited automatically due to
        // System Settings' current hybrid QtWidgets/QtQuick infrastructure.
        Rectangle {
            id: topOverlay
            color: LingmoUI.Theme.backgroundColor
            height: LingmoUI.Units.largeSpacing
            width: 1
        }

        Rectangle {
            id: bottomOverlay
            y: sideBar.pageStack.currentItem?.header.height - LingmoUI.Units.largeSpacing
            color: LingmoUI.Theme.backgroundColor
            height: LingmoUI.Units.largeSpacing - 1
            width: 1
        }
    }
}
