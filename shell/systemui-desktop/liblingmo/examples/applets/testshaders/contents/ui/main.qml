/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

Item {
    id: root
    width: 400
    height: 400

    Layout.minimumWidth: LingmoUI.Units.gridUnit * 20
    Layout.minimumHeight: LingmoUI.Units.gridUnit * 30
    property int _s: LingmoUI.Units.iconSizes.small
    property int _h: LingmoUI.Units.iconSizes.desktop
    property int _m: 12

    Item {
        id: mainItem
        anchors.fill: parent

        LingmoComponents.TabBar {
            id: tabBar

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: LingmoUI.Units.iconSizes.toolbar * 1.5

            LingmoComponents.TabButton { tab: colorShower; text: tab.pageName; }
            LingmoComponents.TabButton { tab: wobbleExample; text: tab.pageName; }
            LingmoComponents.TabButton { tab: shadowExample; text: tab.pageName; }
            LingmoComponents.TabButton { tab: simpleExample; text: tab.pageName; }
            //LingmoComponents.TabButton { tab: vertexPage; iconSource: vertexPage.icon; }
        }

        LingmoComponents.TabGroup {
            id: tabGroup
            anchors {
                left: parent.left
                right: parent.right
                top: tabBar.bottom
                bottom: parent.bottom
            }

            ColorShower {
                id: colorShower
            }
            WobbleExample {
                id: wobbleExample
            }
//             ColorExample {
//                 id: colorExample
//             }
            Shadows {
                id: shadowExample
            }
            SimpleExample {
                id: simpleExample
            }
//             EditorPage {
//                 id: vertexPage
//             }
        }
    }

    Component.onCompleted: {
        print("Shader Test Applet loaded");
    }
}
