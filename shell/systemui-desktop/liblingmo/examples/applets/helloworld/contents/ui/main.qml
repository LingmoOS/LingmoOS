/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.plasmoid
import org.kde.lingmo.components as LingmoComponents

PlasmoidItem {
    Layout.minimumWidth: LingmoUI.Units.gridUnit * 5
    Layout.minimumHeight: LingmoUI.Units.gridUnit * 5

    implicitHeight: LingmoUI.Units.gridUnit * 10
    implicitWidth: LingmoUI.Units.gridUnit * 10

    LingmoComponents.Label {
        anchors.fill: parent
        wrapMode: Text.Wrap
        text: i18n("Hello world")
    }
}
