/*
 *   SPDX-FileCopyrightText: 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
 *   SPDX-FileCopyrightText: 2013, 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *   SPDX-FileCopyrightText: 2021 Devin Lin <devin@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.1
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as PC3
import org.kde.lingmo.private.brightnesscontrolplugin as BC

Item {
    id: root

    implicitHeight: brightnessRow.implicitHeight

    BC.ScreenBrightnessControl {
        id: screenBrightnessControl
    }

    RowLayout {
        id: brightnessRow
        spacing: LingmoUI.Units.smallSpacing * 2

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignVCenter
            Layout.leftMargin: LingmoUI.Units.smallSpacing
            Layout.preferredWidth: LingmoUI.Units.iconSizes.smallMedium
            Layout.preferredHeight: width
            source: "low-brightness"
        }

        PC3.Slider {
            id: brightnessSlider
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            from: 1
            to: screenBrightnessControl.brightnessMax
            value: screenBrightnessControl.brightness

            onMoved: {
                screenBrightnessControl.brightness = value;
            }
        }

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignVCenter
            Layout.rightMargin: LingmoUI.Units.smallSpacing
            Layout.preferredWidth: LingmoUI.Units.iconSizes.smallMedium
            Layout.preferredHeight: width
            source: "high-brightness"
        }
    }
}
