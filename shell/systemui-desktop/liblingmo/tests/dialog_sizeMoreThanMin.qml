/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import QtQuick.Controls as Controls
import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore

Item {
    id: root
    Layout.minimumWidth: 300
    Layout.minimumHeight: 300
    Controls.Button {
        id: button
        anchors.centerIn: parent
        text: "Show Dialog"
        onClicked: {
            //changing the minimumHeight of the mainItem of an hidden dialog
            //shouldn't
            rect.Layout.minimumHeight = rect.Layout.minimumHeight + 1
            rect.Layout.minimumWidth = rect.Layout.minimumWidth + 1
            subDialog.visible = !subDialog.visible
        }
    }
    LingmoCore.Dialog {
        id: subDialog
        location: LingmoCore.Types.Floating
        visualParent: button
        visible: false

        Rectangle {
            id: rect
            width: 500
            height: 500
            Layout.minimumWidth: 300
            Layout.minimumHeight: 300

            color: "red"

            Rectangle {
                anchors.centerIn: parent
                width: rect.Layout.minimumWidth
                height: rect.Layout.minimumHeight
                Text {
                    anchors.fill: parent
                    wrapMode: Text.WordWrap
                    text: "you should see a red border around this white area"
                }
            }
        }
    }
}
