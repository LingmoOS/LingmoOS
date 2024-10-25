/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 BusyIndicator"
    contentItem: ColumnLayout {
        spacing: LingmoUI.Units.gridUnit

        LingmoComponents.Label {
            wrapMode: Text.WordWrap
            text: "The BusyIndicator should have a height of 16px and should have a 1:1 aspect ratio"
            Layout.preferredWidth: Math.max(busyIndicatorLayout.width, root.implicitHeaderWidth)
        }
        LingmoComponents.BusyIndicator {
            Layout.preferredHeight: 16
        }

        LingmoComponents.Label {
            wrapMode: Text.WordWrap
            text: "The BusyIndicator should use its implicit size."
            Layout.preferredWidth: Math.max(busyIndicatorLayout.width, root.implicitHeaderWidth)
        }
        LingmoComponents.BusyIndicator {}

        LingmoComponents.Label {
            wrapMode: Text.WordWrap
            text: "The BusyIndicator should maintain a 1:1 aspect ratio, disappear when unchecked and restart when checked."
            Layout.preferredWidth: Math.max(busyIndicatorLayout.width, root.implicitHeaderWidth)
        }

        RowLayout {
            id: busyIndicatorLayout
            spacing: LingmoUI.Units.gridUnit

            LingmoComponents.BusyIndicator {
                Layout.fillWidth: true
                Layout.fillHeight: true
                running: runningButton.checked
            }

            LingmoComponents.CheckBox {
                id: runningButton
                text: "Running"
                checked: true
            }
        }
    }
}
