/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

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
    PlasmaCore.Dialog {
        id: subDialog
        location: PlasmaCore.Types.Floating
        visualParent: button
        visible: false

        Rectangle {
            id: rect
            Layout.minimumWidth: 300
            Layout.minimumHeight: 300
            Layout.maximumHeight: 500
            property int fixedHeight: 500
            width: 500
            height: fixedHeight

            color: "red"
            border {
                color: "blue"
                width: 3
            }

            Controls.Button {
                text: "Resize"
                anchors.centerIn: parent
                onClicked: {
                    rect.fixedHeight = rect.Layout.minimumHeight = rect.Layout.maximumHeight = (rect.fixedHeight == 500 ? rect.fixedHeight = 100 : rect.fixedHeight = 500)

                    //subDialog.height = (subDialog.height == 500 ? subDialog.height = 100 : subDialog.height = 500)
                }
            }
        }
    }
}
