/*
   SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.5
import QtQuick.Layouts 1.1

import org.kde.lingmoui 2.5 as LingmoUI

Rectangle {
    id: root
    color: LingmoUI.Theme.backgroundColor
    property int verticalMargin: LingmoUI.Units.gridUnit * 3

    RowLayout {
        anchors {
            bottom: parent.verticalCenter
            bottomMargin: verticalMargin
            horizontalCenter: parent.horizontalCenter
        }
        spacing: LingmoUI.Units.largeSpacing

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignHCenter
            source: "preferences-system"
            implicitWidth: LingmoUI.Units.iconSizes.enormous
            implicitHeight: LingmoUI.Units.iconSizes.enormous
        }


        ColumnLayout {
            RowLayout {
                LingmoUI.Icon {
                    Layout.alignment: Qt.AlignHCenter
                    source: Qt.resolvedUrl("qrc:/qt/qml/org/kde/systemsettings/lingmo-logo.svg")
                    color: LingmoUI.Theme.textColor
                    isMask: true
                    implicitWidth: LingmoUI.Units.iconSizes.medium
                    implicitHeight: LingmoUI.Units.iconSizes.medium
                }
                LingmoUI.Heading {
                    text: i18n("Lingmo")
                    level: 1
                    font.weight: Font.Bold
                }
            }

            LingmoUI.Heading {
                text: i18n("System Settings")
                level: 1
            }
        }
    }
}
