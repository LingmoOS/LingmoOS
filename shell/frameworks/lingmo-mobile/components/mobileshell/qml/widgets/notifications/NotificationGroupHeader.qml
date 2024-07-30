/*
 * SPDX-FileCopyrightText: 2021 Devin Lin <devin@kde.org>
 * SPDX-FileCopyrightText: 2018-2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.8
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2

import org.kde.lingmoui 2.12 as LingmoUI

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.lingmo5support 2.0 as P5Support
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.notificationmanager as NotificationManager

import org.kde.coreaddons 1.0 as KCoreAddons

RowLayout {
    id: notificationHeading

    property var applicationIconSource
    property string applicationName
    property string originName

    spacing: LingmoUI.Units.smallSpacing
    Layout.preferredHeight: Math.max(applicationNameLabel.implicitHeight, LingmoUI.Units.iconSizes.small)

    LingmoUI.Icon {
        id: applicationIconItem
        Layout.topMargin: LingmoUI.Units.smallSpacing
        Layout.bottomMargin: LingmoUI.Units.smallSpacing
        Layout.preferredWidth: LingmoUI.Units.iconSizes.small
        Layout.preferredHeight: LingmoUI.Units.iconSizes.small
        source: notificationHeading.applicationIconSource
        visible: valid
    }

    LingmoComponents.Label {
        id: applicationNameLabel
        Layout.leftMargin: LingmoUI.Units.smallSpacing
        Layout.fillWidth: true
        opacity: 0.6
        textFormat: Text.PlainText
        elide: Text.ElideLeft
        font.bold: true
        text: notificationHeading.applicationName + (notificationHeading.originName ? " · " + notificationHeading.originName : "")
    }

    Item {
        id: spacer
        Layout.fillWidth: true
    }
}
