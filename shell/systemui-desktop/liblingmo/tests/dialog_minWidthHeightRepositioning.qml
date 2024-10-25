/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore

LingmoCore.Dialog {
    id: root
    location: LingmoCore.Types.Floating

    Rectangle {
        id: rect
        Layout.minimumWidth: 300
        Layout.minimumHeight: 300

        color: "red"

        Rectangle {
            width: rect.Layout.minimumWidth
            height: rect.Layout.minimumHeight
        }
        ColumnLayout {
            anchors.top: parent.top
            Controls.Label {
                Layout.maximumWidth: rect.Layout.minimumWidth
                text: "Use Alt + Left Click to move the window to a side and then increase the minWidth/Height. The window should reposition itself"
                wrapMode: Text.WordWrap
            }
            Controls.Button {
                text: "Increase MinWidth"
                onClicked: {
                    rect.Layout.minimumWidth = rect.Layout.minimumWidth + 10
                }
            }
            Controls.Button {
                text: "Increase MinHeight"
                onClicked: {
                    rect.Layout.minimumHeight = rect.Layout.minimumHeight + 10
                }
            }
            Controls.Button {
                text: "Increase dialog width"
                onClicked: {
                    root.width = root.width + 10
                }
            }
            Controls.Button {
                text: "Increase dialog height"
                onClicked: {
                    root.height = root.height + 10
                }
            }
        }
    }
}
