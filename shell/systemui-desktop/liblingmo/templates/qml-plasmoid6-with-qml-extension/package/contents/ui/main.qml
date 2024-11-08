/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.plasmoid
import org.kde.lingmo.components as LingmoComponents

import org.kde.lingmo.private.%{APPNAMELC} 1.0

Item {
    Plasmoid.fullRepresentation: ColumnLayout {
        anchors.fill: parent
        LingmoComponents.Label {
            Layout.alignment: Qt.AlignCenter
            text: HelloWorld.text
        }
    }
}
