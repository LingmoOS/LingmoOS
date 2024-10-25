/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import QtQuick.Layouts

import org.kde.lingmo.core as LingmoCore

LingmoCore.Dialog {
    visible: true

    mainItem: Item {
        width: 200
        height: 200

        MouseArea {
            anchors.fill: parent
            onClicked: Qt.quit()
        }
    }
}
