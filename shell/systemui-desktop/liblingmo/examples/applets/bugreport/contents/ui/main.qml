/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.lingmo.plasmoid
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

PlasmoidItem {
    id: root

    switchWidth: LingmoUI.Units.gridUnit * 10
    switchHeight: LingmoUI.Units.gridUnit * 5

    Plasmoid.icon: "tools-report-bug"

    fullRepresentation: LingmoComponents.ScrollView {
        Layout.minimumWidth: LingmoUI.Units.gridUnit * 10
        Layout.preferredWidth: LingmoUI.Units.gridUnit * 20

        LingmoComponents.ScrollBar.horizontal.policy: LingmoComponents.ScrollBar.AlwaysOff

        contentWidth: availableWidth
        contentHeight: Math.ceil(col.implicitHeight + col.anchors.margins * 2)

        Item {
            width: parent.width

            LingmoUI.Icon {
                id: icon

                anchors {
                    top: col.top
                    right: col.right
                }
                width: LingmoUI.Units.iconSizes.large
                height: LingmoUI.Units.iconSizes.large
                source: root.Plasmoid.icon
            }

            ColumnLayout {
                id: col

                anchors {
                    fill: parent
                    margins: LingmoUI.Units.gridUnit
                }
                spacing: LingmoUI.Units.largeSpacing

                LingmoUI.Heading {
                    level: 1
                    text: i18n("Reporting Bugs")
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    Layout.rightMargin: icon.width
                }
                LingmoUI.Heading {
                    level: 3
                    text: i18n("So you found something wrong in Lingmo…")
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    Layout.rightMargin: icon.width
                }
                LingmoComponents.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.StyledText
                    text: i18n("You are running a development version of Lingmo. This software is not fit for production use. We do, however encourage testing and reporting the results. A few easy steps to report a bug: <br />\
            <ul>\
            <li>Check <a href=\"\">here if the bug already exists</li>\
            <li>Report it using the form <a href=\"\">here</li>\
            <li>Be ready to provide us feedback, so we can debug the problem</li>\
            </ul>\
            <br />If you would like to participate in development, or have a question, you can ask them on the lingmo-devel@kde.org mailing list.\
             ")
                }
            }
        }
    }
}
