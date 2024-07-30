/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

GenericPage {
    heading: i18nc("@info:window", "Powerful When Needed")
    description: xi18nc("@info:usagetip", "Plasma is an extremely feature-rich environment, designed to super-charge your productivity! Here is just a smattering of the things it can do for you:")

    ColumnLayout {
        id: layout
        anchors.fill: parent

        spacing: Kirigami.Units.largeSpacing

        GridLayout {
            id: grid

            readonly property int columnsforHorizontalLayout: 3
            readonly property int columnsforVerticalLayout: 2
            readonly property int cellWidth: Math.round(layout.width / columns)
            readonly property int cellHeight: Math.max(overview.implicitHeight,
                                                       krunner.implicitHeight,
                                                       kdeconnect.implicitHeight,
                                                       activities.implicitHeight,
                                                       vaults.implicitHeight,
                                                       systemsettings.implicitHeight)
            readonly property int spaceForTitles: Math.round(layout.width / columnsforHorizontalLayout) - vaults.fixedSizeStuff - (columnSpacing * (columnsforHorizontalLayout - 1))
            readonly property bool verticalLayout: (overview.implicitTitleWidth > spaceForTitles)
                                                || (krunner.implicitTitleWidth > spaceForTitles)
                                                || (kdeconnect.implicitTitleWidth > spaceForTitles)
                                                || (activities.implicitTitleWidth > spaceForTitles)
                                                || (vaults.implicitTitleWidth > spaceForTitles)
                                                || (systemsettings.implicitTitleWidth > spaceForTitles)

            rows: verticalLayout ? 6 : 2
            columns: verticalLayout ? columnsforVerticalLayout : columnsforHorizontalLayout
            rowSpacing: Kirigami.Units.smallSpacing
            columnSpacing: Kirigami.Units.smallSpacing

            // First row
            PlasmaFeatureButton {
                id: overview
                Layout.fillWidth: true
                Layout.maximumWidth: grid.cellWidth
                Layout.preferredHeight: grid.cellHeight
                title: i18nc("@title:row Name of the 'Overview' KWin effect", "Overview")
                subtitle: i18nc("@info Caption for Overview button", "Your system command center")
                buttonIcon: "kwin"
                onClicked: pageStack.layers.push(app._createPage("Overview.qml"))
            }

            PlasmaFeatureButton {
                id: krunner
                Layout.fillWidth: true
                Layout.maximumWidth: grid.cellWidth
                Layout.preferredHeight: grid.cellHeight
                title: i18nc("@title:row", "KRunner")
                subtitle: i18nc("@info Caption for KRunner button", "Search for anything")
                buttonIcon: "krunner"
                onClicked: pageStack.layers.push(app._createPage("KRunner.qml"))
            }

            PlasmaFeatureButton {
                id: kdeconnect
                Layout.fillWidth: true
                Layout.maximumWidth: grid.cellWidth
                Layout.preferredHeight: grid.cellHeight
                title: i18nc("@title:row Name of the 'KDE Connect' feature", "KDE Connect")
                subtitle: i18nc("@info Caption for KDE Connect button", "Connect your phone and your computer")
                buttonIcon: "kdeconnect"
                onClicked: pageStack.layers.push(app._createPage("KDEConnect.qml"))
            }

            // Second row
            PlasmaFeatureButton {
                id: activities
                Layout.fillWidth: true
                Layout.maximumWidth: grid.cellWidth
                Layout.preferredHeight: grid.cellHeight
                title: i18nc("@title:row Name of the 'Activities' Plasma feature", "Activities")
                subtitle: i18nc("@info Caption for Activities button. Note that 'Separate' is being used as an imperative verb here, not a noun.", "Separate work, school, or home tasks")
                buttonIcon: "preferences-desktop-activities"
                onClicked: pageStack.layers.push(app._createPage("Activities.qml"))
            }

            PlasmaFeatureButton {
                id: vaults
                Layout.fillWidth: true
                Layout.maximumWidth: grid.cellWidth
                Layout.preferredHeight: grid.cellHeight
                title: i18nc("@title:row Short form of the 'Vaults' Plasma feature", "Vaults")
                subtitle: i18nc("@info Caption for Plasma Vaults button", "Store sensitive files securely")
                buttonIcon: "plasmavault"
                onClicked: pageStack.layers.push(app._createPage("Vaults.qml"))
            }

            PlasmaFeatureButton {
                id: systemsettings
                Layout.fillWidth: true
                Layout.maximumWidth: grid.cellWidth
                Layout.preferredHeight: grid.cellHeight
                title: i18nc("@title:row", "System Settings")
                subtitle: i18nc("@info Caption for Get System Settings button", "Configure the system")
                buttonIcon: "systemsettings"
                onClicked: pageStack.layers.push(app._createPage("SystemSettings.qml"))
            }
        }

        Kirigami.UrlButton {
            text: i18nc("@action:button", "Learn about more Plasma features")
            url: "https://userbase.kde.org/Plasma?source=plasma-welcome"
        }
    }
}
