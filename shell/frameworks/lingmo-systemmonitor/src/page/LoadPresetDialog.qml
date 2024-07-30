/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

LingmoUI.Dialog {
    id: dialog

    property QtObject controller

    readonly property string selectedTitle: presetList.currentItem ? presetList.currentItem.title : ""
    readonly property string selectedFace: presetList.currentItem ? presetList.currentItem.chartFace : ""
    readonly property string selectedPreset: presetList.currentItem ? presetList.currentItem.preset : ""

    title: i18nc("@title:window", "Load Preset")

    preferredWidth: LingmoUI.Units.gridUnit * 20
    preferredHeight: LingmoUI.Units.gridUnit * 30

    focus: true

    // We already have a cancel button in the footer
    showCloseButton: false

    standardButtons: Dialog.Cancel

    customFooterActions: [
        LingmoUI.Action {
            text: i18nc("@action:button", "Load")
            icon.name: "document-open"
            enabled: presetList.currentIndex != -1
            onTriggered: dialog.accept()
        }
    ]

    ListView {
        id: presetList

        currentIndex: -1
        clip: true

        model: dialog.controller.availablePresetsModel
        delegate: LingmoUI.SwipeListItem {
            highlighted: ListView.isCurrentItem

            property string title: model.display
            property string chartFace: model.config && model.config.chartFace !== undefined ? model.config.chartFace : ""
            property string preset: model.pluginId

            contentItem: Label {
                Layout.fillWidth: true
                text: model.display
            }
            actions: LingmoUI.Action {
                icon.name: "delete"
                visible: model.writable
                onTriggered: dialog.controller.uninstallPreset(model.pluginId);
            }
            onClicked: {
                presetList.currentIndex = index
            }
        }
    }
}
