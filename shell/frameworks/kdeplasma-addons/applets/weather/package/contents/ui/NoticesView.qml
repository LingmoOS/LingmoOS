/*
 * SPDX-FileCopyrightText: 2018 Friedrich W. H. Kossebau <kossebau@kde.org>
 * SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

ListView {
    id: root

    anchors.fill: parent
    boundsBehavior: Flickable.StopAtBounds

    delegate: RowLayout {
        width: ListView.view.width - (scrollBar.visible ? scrollBar.width : 0)
        spacing: 0

        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: Kirigami.Units.gridUnit * 5
            Layout.maximumWidth: Kirigami.Units.gridUnit * 10
            Layout.margins: Kirigami.Units.largeSpacing

            text: modelData.timestamp
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }

        Kirigami.Icon {
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: implicitWidth
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            source: (modelData.priority >= 3) ? 'flag-red-symbolic' :
                    (modelData.priority >= 2) ? 'flag-yellow-symbolic' :
                                                'flag-blue-symbolic'
        }

        Kirigami.SelectableLabel {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: Kirigami.Units.gridUnit * 5
            Layout.margins: Kirigami.Units.largeSpacing

            text: modelData.description
            wrapMode: Text.Wrap
        }

        PlasmaComponents.ToolButton {
            visible: !!modelData.infoUrl
            Layout.alignment: Qt.AlignTop
            Layout.minimumWidth: implicitWidth
            icon.name: 'showinfo-symbolic'
            text: i18nc("@action:button", "Show more information")
            display: PlasmaComponents.ToolButton.IconOnly
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

    Kirigami.Separator {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        visible: !root.atYBeginning
    }

    Kirigami.Separator {
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        visible: !root.atYEnd
    }
}
