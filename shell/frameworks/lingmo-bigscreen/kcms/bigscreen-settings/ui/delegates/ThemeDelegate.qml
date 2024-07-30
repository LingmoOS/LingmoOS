/*
    SPDX-FileCopyrightText: 2019 Aditya Mehra <aix.m@outlook.com>
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.14
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.14
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui as LingmoUI
import org.kde.mycroft.bigscreen 1.0 as BigScreen
import Qt5Compat.GraphicalEffects

BigScreen.AbstractDelegate {
    id: delegate
    implicitWidth: listView.cellWidth * 2
    implicitHeight: textLayout.implicitHeight + nameLabel.height + LingmoUI.Units.largeSpacing * 2


    Behavior on implicitWidth {
        NumberAnimation {
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    contentItem: Item {
        id: connectionItemLayout

        ColumnLayout {
            id: textLayout

            anchors {
                fill: parent
                leftMargin: LingmoUI.Units.smallSpacing
            }

            Item {
                id: connectionSvgIcon
                width: parent.width
                height: width
                ThemePreview {
                    id: preview
                    anchors.fill: parent
                    themeName: model.pluginNameRole
                }
            }

            LingmoComponents.Label {
                id: nameLabel
                Layout.fillWidth: true
                visible: text.length > 0
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 2
                color: LingmoUI.Theme.textColor
                textFormat: Text.PlainText
                text: model.display
            }
        }

        LingmoUI.Icon {
            id: dIcon
            anchors.bottom: parent.bottom
            anchors.bottomMargin: -LingmoUI.Units.smallSpacing
            anchors.right: parent.right
            anchors.rightMargin: -LingmoUI.Units.smallSpacing
            width: LingmoUI.Units.iconSizes.smallMedium
            height: width
            source: Qt.resolvedUrl("../images/green-tick-thick.svg")
            visible:  kcm.themeName === model.packageNameRole
        }
    }

    Keys.onReturnPressed: clicked()

    onClicked: {
        setTheme(model.packageNameRole)
    }
}
