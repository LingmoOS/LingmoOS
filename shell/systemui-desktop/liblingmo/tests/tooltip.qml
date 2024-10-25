/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.core as LingmoCore

Rectangle {
    color: "white"
    width: 600
    height: 800

    Column {
        anchors.centerIn: parent
        spacing: 4

        LingmoCore.ToolTipArea {
            width: 300
            height: 50

            mainText: "This is some really really really really long text that should be truncated"
            subText: "subtext"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "long tooltip"
            }

        }

        LingmoCore.ToolTipArea {
            width: 300
            height: 50

            mainText: "A"
            subText: "B"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "short tooltip"
            }
        }

        LingmoCore.ToolTipArea {
            width: 300
            height: 50

            mainText: "A"
            subText: "Lorem ipsum dolor sit amet, consectetur adipiscing spaghetti italiano random cheesecake blah blah"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "long subtext"
            }
        }

        LingmoCore.ToolTipArea {
            width: 300
            height: 50

	    active: false

            mainText: "A"
            subText: "B"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "tooltip exists but inactive"
            }
        }

        LingmoCore.ToolTipArea {
            width: 300
            height: 50

            icon: "document-edit"
            mainText: "A title"
            subText: "A subtext"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "with icon"
            }
        }

        LingmoCore.ToolTipArea {
            width: 300
            height: 50

            image: "/usr/share/icons/oxygen/128x128/devices/multimedia-player.png"
            mainText: "A title"
            subText: "A subtext"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "with image"
            }
        }

        LingmoCore.ToolTipArea {
            width: 300
            height: 50

            icon: "kde"
            mainText: "Notifications"
            subText: "Display notifications and jobs"

            Rectangle {
                color: "red"
                anchors.fill: parent
            }

            Text {
                anchors.fill: parent
                text: "Notification applet tooltip"
            }
        }

    }
}

