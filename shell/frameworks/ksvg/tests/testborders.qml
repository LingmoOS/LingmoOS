/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15

import org.kde.lingmo.core as LingmoCore

Item
{
    width: 500
    height: 500

    LingmoCore.FrameSvgItem {
        id: theItem

        imagePath: "widgets/background"
        anchors {
            fill: parent
            margins: 10
        }

        Button {
            text: "left"
            checkable: true
            checked: true
            anchors {
                horizontalCenterOffset: -50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= LingmoCore.FrameSvg.LeftBorder;
                else
                    theItem.enabledBorders &=~LingmoCore.FrameSvg.LeftBorder;
            }
        }
        Button {
            text: "right"
            checkable: true
            checked: true

            anchors {
                horizontalCenterOffset: 50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= LingmoCore.FrameSvg.RightBorder;
                else
                    theItem.enabledBorders &=~LingmoCore.FrameSvg.RightBorder;
            }
        }
        Button {
            text: "top"
            checkable: true
            checked: true

            anchors {
                verticalCenterOffset: -50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= LingmoCore.FrameSvg.TopBorder;
                else
                    theItem.enabledBorders &=~LingmoCore.FrameSvg.TopBorder;
            }
        }
        Button {
            text: "bottom"
            checkable: true
            checked: true

            anchors {
                verticalCenterOffset: 50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= LingmoCore.FrameSvg.BottomBorder;
                else
                    theItem.enabledBorders &=~LingmoCore.FrameSvg.BottomBorder;
            }
        }
    }
}

