/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore

ColumnLayout {
    Controls.Label {
        Layout.maximumWidth: mainLayout.width
        wrapMode: Text.WordWrap
        text: "Hover over every rectangle so that the tooltip pops up. It should popup in the correct position"
    }

    RowLayout {
        id: mainLayout
        Rectangle {
            width: 300
            height: 100
            color: "red"

            LingmoCore.ToolTipArea {
                width: 300
                height: 50

                mainText: "Title Number 1"
                subText: "subtext"
                icon: "lingmo"
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "blue"

            LingmoCore.ToolTipArea {
                width: 500
                height: 110

                mainText: "Title Number 2"
                subText: "This is some really really really long subtext. So lets write stores about the woods and the trees and how we're going hiking. Yaye!"
                icon: "configure"
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "green"

            LingmoCore.ToolTipArea {
                width: 350
                height: 70

                mainText: "Wakka Wakka"
                subText: "It's time for Africa"
            }
        }
    }
}
