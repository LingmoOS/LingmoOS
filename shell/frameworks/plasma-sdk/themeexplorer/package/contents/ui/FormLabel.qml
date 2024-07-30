/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15


Label {
    Layout.alignment: Qt.AlignRight
    property Item buddy
    MouseArea {
        anchors.fill: parent
        onClicked: buddy.focus = true
    }
}

