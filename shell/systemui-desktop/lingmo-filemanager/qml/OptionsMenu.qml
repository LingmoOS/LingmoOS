/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import LingmoUI 1.0 as LingmoUI

LingmoUI.DesktopMenu {
    id: control

    MenuItem {
        Layout.fillWidth: true

        Image {
            id: _gridIcon
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/grid.svg" : "qrc:/images/light/grid.svg"
            sourceSize: Qt.size(width, height)
            width: 22
            height: width
            smooth: false
        }

        Text {
            anchors.left: _gridIcon.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            text: qsTr("Icons")
            color: LingmoUI.Theme.textColor
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/checked.svg" : "qrc:/images/light/checked.svg"
            sourceSize: Qt.size(width, height)
            width: 22
            height: width
            visible: settings.viewMethod === 1
            smooth: false
        }

        onTriggered: settings.viewMethod = 1
    }

    MenuItem {
        Layout.fillWidth: true

        Image {
            id: _listIcon
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/list.svg" : "qrc:/images/light/list.svg"
            sourceSize: Qt.size(width, height)
            width: 22
            height: width
            smooth: false
        }

        Text {
            anchors.left: _listIcon.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            text: qsTr("List")
            color: LingmoUI.Theme.textColor
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/checked.svg" : "qrc:/images/light/checked.svg"
            sourceSize: Qt.size(width, height)
            width: 22
            height: width
            visible: settings.viewMethod === 0
            smooth: false
        }

        onTriggered: settings.viewMethod = 0
    }

    MenuSeparator {
        Layout.fillWidth: true
    }

    MenuItem {
        Layout.fillWidth: true

//        Image {
//            id: orderByNameIcon
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.left: parent.left
//            anchors.leftMargin: LingmoUI.Units.largeSpacing
//            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/order_by_name.svg" : "qrc:/images/light/order_by_name.svg"
//            sourceSize: Qt.size(width, height)
//            width: 22
//            height: width
//            smooth: false
//        }

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            text: qsTr("Name")
            color: LingmoUI.Theme.textColor
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/checked.svg" : "qrc:/images/light/checked.svg"
            sourceSize: Qt.size(width, height)
            height: width
            width: 22
            visible: settings.sortMode === 0
            smooth: false
        }

        onTriggered: settings.sortMode = 0
    }

    MenuItem {
        Layout.fillWidth: true

//        Image {
//            id: orderByDateIcon
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.left: parent.left
//            anchors.leftMargin: LingmoUI.Units.largeSpacing
//            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/date.svg" : "qrc:/images/light/date.svg"
//            sourceSize: Qt.size(width, height)
//            width: 22
//            height: width
//            smooth: false
//        }

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            text: qsTr("Date")
            color: LingmoUI.Theme.textColor
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/checked.svg" : "qrc:/images/light/checked.svg"
            sourceSize: Qt.size(width, height)
            width: 22
            height: width
            visible: settings.sortMode === 2
            smooth: false
        }

        onTriggered: settings.sortMode = 2
    }

    MenuItem {
        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            text: qsTr("Type")
            color: LingmoUI.Theme.textColor
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/checked.svg" : "qrc:/images/light/checked.svg"
            sourceSize: Qt.size(width, height)
            width: 22
            height: width
            visible: settings.sortMode === 6
            smooth: false
        }

        onTriggered: settings.sortMode = 6
    }

    MenuItem {
        Layout.fillWidth: true

//        Image {
//            id: orderBySizeIcon
//            anchors.verticalCenter: parent.verticalCenter
//            anchors.left: parent.left
//            anchors.leftMargin: LingmoUI.Units.largeSpacing
//            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/size.svg" : "qrc:/images/light/size.svg"
//            sourceSize: Qt.size(width, height)
//            width: 22
//            height: width
//            smooth: false
//        }

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: LingmoUI.Units.largeSpacing
            text: qsTr("Size")
            color: LingmoUI.Theme.textColor
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: LingmoUI.Units.largeSpacing * 1.5
            source: LingmoUI.Theme.darkMode ? "qrc:/images/dark/checked.svg" : "qrc:/images/light/checked.svg"
            sourceSize: Qt.size(width, height)
            width: 22
            height: width
            visible: settings.sortMode === 1
            smooth: false
        }

        onTriggered: settings.sortMode = 1
    }
}
