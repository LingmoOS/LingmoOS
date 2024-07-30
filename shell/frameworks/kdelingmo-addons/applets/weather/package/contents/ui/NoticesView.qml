/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ListView {
    id: root

    anchors.fill: parent
    boundsBehavior: Flickable.StopAtBounds

    delegate: RowLayout {
        width: ListView.view.width - (scrollBar.visible ? scrollBar.width : 0)
        spacing: 0

        LingmoComponents.Label {
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: LingmoUI.Units.gridUnit * 5
            Layout.maximumWidth: LingmoUI.Units.gridUnit * 10
            Layout.margins: LingmoUI.Units.largeSpacing

            text: modelData.timestamp
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: implicitWidth
            implicitWidth: LingmoUI.Units.iconSizes.smallMedium
            source: (modelData.priority >= 3) ? 'flag-red-symbolic' :
                    (modelData.priority >= 2) ? 'flag-yellow-symbolic' :
                                                'flag-blue-symbolic'
        }

        LingmoUI.SelectableLabel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: LingmoUI.Units.gridUnit * 5
            Layout.margins: LingmoUI.Units.largeSpacing

            text: modelData.description
            wrapMode: Text.Wrap
        }

        LingmoComponents.ToolButton {
            visible: !!modelData.infoUrl
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: implicitWidth
            icon.name: 'showinfo-symbolic'
            text: i18nc("@action:button", "Show more information")
            display: LingmoComponents.ToolButton.IconOnly
            onClicked: {
                Qt.openUrlExternally(Qt.resolvedUrl(modelData.infoUrl))
            }
        }
    }

    QQC2.ScrollBar.vertical: QQC2.ScrollBar {
        id: scrollBar
        anchors.right: parent.right
        visible: root.contentHeight > root.height
    }

    LingmoUI.Separator {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        visible: !root.atYBeginning
    }

    LingmoUI.Separator {
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        visible: !root.atYEnd
    }
}
