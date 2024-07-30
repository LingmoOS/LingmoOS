/*
 * SPDX-FileCopyrightText: 2022 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2023 Nate Graham <nate@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

import org.kde.ksysguard.page
import org.kde.ksysguard.faces as Faces

LingmoUI.Dialog {
    id: dialog

    property var missingSensors: { }
    property var sensorReplacement: { }

    title: i18nc("@title", "Replace Missing Sensors")

    preferredWidth: LingmoUI.Units.gridUnit * 30
    preferredHeight: LingmoUI.Units.gridUnit * 30

    focus: true

    // We already have a cancel button in the footer
    showCloseButton: false

    standardButtons: Dialog.Save | Dialog.Cancel

    onAccepted: {
        let result = []

        for (let i = 0; i < sensorsModel.count; ++i) {
            let entry = sensorsModel.get(i)
            if (!entry.replacement) {
                continue
            }

            result.push({
                "face": entry.face,
                "sensor": entry.sensor,
                "replacement": entry.replacement
            })
        }

        sensorReplacement = result
    }

    onMissingSensorsChanged: {
        sensorsModel.clear();
        missingSensors.sort((first, second) => first.title.localeCompare(second.title))
        for (let entry of missingSensors) {
            entry.replacement = ""
            sensorsModel.append(entry)
        }
    }

    ListView {
        model: ListModel { id: sensorsModel }

        section.property: "title"
        section.delegate: LingmoUI.ListSectionHeader {
            required property string section
            width: ListView.view.width
            label: section
        }

        clip: true

        delegate: ItemDelegate {
            id: delegate

            width: ListView.view.width

            text: model.sensor

            contentItem: RowLayout {
                spacing: LingmoUI.Units.smallSpacing

                Label {
                    Layout.fillWidth: true
                    text: delegate.text
                    textFormat: Text.MarkdownText
                }

                Faces.Choices {
                    Layout.preferredWidth: delegate.width * 0.5

                    supportsColors: false
                    maxAllowedSensors: 1
                    labelsEditable: false
                    labels: { }

                    onSelectedChanged: {
                        if (selected.length > 0) {
                            sensorsModel.setProperty(index, "replacement", selected[0])
                        }
                    }
                }
            }
        }
    }
}

