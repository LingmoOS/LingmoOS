/*
 * Rewrite of the KDE4-Lingmo Binary Clock for KF5/Lingmo/QML
 *
 * SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
 * SPDX-FileCopyrightText: 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 * SPDX-FileCopyrightText: 2023 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
 *
 * Original code (KDE4):
 * SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
 * SPDX-FileCopyrightText: 2007 Davide Bettio <davide.bettio@kdemail.net>
 *
 * Based on FuzzyClock.qml:
 * SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>
 * SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui as LingmoUI

Item {
    id: main

    readonly property real w1: (main.height-5*LingmoUI.Units.smallSpacing)*dots/4

    Layout.minimumWidth: w1 < 20 ? 20 : w1
    Layout.maximumWidth: Infinity
    Layout.preferredWidth: Layout.minimumWidth

    Layout.minimumHeight: 16+(LingmoUI.Units.smallSpacing*5)
    //Layout.maximumHeight: vertical ? Layout.minimumHeight : Infinity
    //Layout.preferredHeight: Layout.minimumHeight

    readonly property int formFactor: plasmoid.formFactor

    readonly property bool constrained: formFactor == LingmoCore.Types.Vertical || formFactor == LingmoCore.Types.Horizontal

    readonly property bool showSeconds: root.showSeconds

    readonly property int hours: root.hours
    readonly property int minutes: root.minutes
    readonly property int seconds: root.seconds

    readonly property int base: 10

    readonly property bool showOffLeds: plasmoid.configuration.showOffLeds

    readonly property int dots: showSeconds ? 6 : 4

    readonly property color onColor: plasmoid.configuration.useCustomColorForActive ? plasmoid.configuration.customColorForActive : LingmoUI.Theme.textColor
    readonly property color offColor: plasmoid.configuration.useCustomColorForInactive ? plasmoid.configuration.customColorForInactive : Qt.rgba(onColor.r, onColor.g, onColor.b, 0.2)

    readonly property int dotSize: Math.min((height-5*LingmoUI.Units.smallSpacing)/4, (width-(dots+1)*LingmoUI.Units.smallSpacing)/dots)

    property bool wasExpanded: false

    readonly property alias mouseArea: mouseArea

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onPressed: wasExpanded = root.expanded
        onClicked: root.expanded = !wasExpanded
    }

    GridLayout {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        columns: main.showSeconds ? 6 : 4
        Repeater {
            model: [8, 4, 2, 1]
            Repeater {
                model: [hours/base, hours%base, minutes/base, minutes%base, seconds/base, seconds%base]
                property var bit: modelData
                Rectangle {
                    property var timeVal: modelData
                    visible: main.dotSize >= 0 && (main.showSeconds || index < 4)
                    width: main.dotSize
                    height: width
                    radius: width/2
                    color: (timeVal & bit) ? main.onColor : (main.showOffLeds ? main.offColor : "transparent")
                }
            }
        }
    }
}
