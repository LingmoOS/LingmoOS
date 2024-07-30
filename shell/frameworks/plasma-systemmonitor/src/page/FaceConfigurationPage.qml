/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


import org.kde.kirigami as Kirigami
import org.kde.newstuff as NewStuff

import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.page

Kirigami.ScrollablePage {
    id: page

    property FaceLoader loader

    property int activeSection: 0

    title: i18nc("@title:window %1 is face name", "Configure %1", loader.controller.title)

    Kirigami.ColumnView.fillWidth: false

    actions: [
        Kirigami.Action {
            icon.name: "dialog-close"
            text: i18nc("@action", "Close")
            onTriggered: applicationWindow().pageStack.pop()

            displayHint: Kirigami.DisplayHint.KeepVisible
        },

        Kirigami.Action {
            text: i18nc("@action", "Load Preset…")
            icon.name: "document-new-from-template"
            onTriggered: loadPresetDialog.open()
        },

        NewStuff.Action {
            text: i18nc("@action", "Get new presets…")
            configFile: "systemmonitor-presets.knsrc"
            pageStack: applicationWindow().pageStack.layers
            onEntryEvent: (entry, event) => {
                if (event == NewStuff.Engine.StatusChangedEvent) {
                    Qt.callLater(loader.controller.availablePresetsModel.reload)
                }
            }
        },

        Kirigami.Action {
            text: i18nc("@action", "Save Settings as Preset")
            icon.name: "document-save-as-template"
            onTriggered: {
                loader.controller.savePreset()
                message.text = i18nc("@info:status %1 is preset name", "Saved settings as preset %1.", loader.controller.title)
                message.visible = true
            }
        },

        Kirigami.Action {
            text: i18nc("@action", "Add Chart as Desktop Widget")
            icon.name: "document-export"
            enabled: WidgetExporter.plasmashellAvailable
            onTriggered: {
                WidgetExporter.exportAsWidget(loader)
            }
        },

        Kirigami.Action { separator: true },

        NewStuff.Action {
            text: i18nc("@action", "Get new display styles…")
            configFile: "systemmonitor-faces.knsrc"
            pageStack: applicationWindow().pageStack.layers
            onEntryEvent: (entry, event) => {
                if (event == NewStuff.Engine.StatusChangedEvent) {
                    Qt.callLater(loader.controller.availableFacesModel.reload)
                }
            }
        }
    ]

    DialogLoader {
        id: loadPresetDialog
        sourceComponent: LoadPresetDialog {

            controller: loader.controller

            onAccepted: {
                loader.controller.title = selectedTitle;
                if (selectedFace) {
                    loader.controller.faceId = selectedFace
                }
                loader.controller.loadPreset(selectedPreset);
                message.text = i18nc("@info:status %1 is preset name", "Loaded preset %1.", selectedTitle);
                message.visible = true
            }
        }
    }

    ColumnLayout {
        id: layout

        Kirigami.InlineMessage {
            id: message
            Layout.fillWidth: true
            showCloseButton: true
        }

        Label {
            Layout.fillWidth: true
            text: i18nc("@label:textbox", "Title")

            CheckBox {
                id: showTitleCheckbox
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                text: i18nc("@option:check", "Show Title")
                checked: page.loader.controller.showTitle
                onToggled: page.loader.controller.showTitle = checked
            }
        }

        TextField {
            id: titleField

            text: page.loader.controller.title

            Layout.fillWidth: true

            onTextEdited: page.loader.controller.title = text
        }

        Label {
            Layout.fillWidth: true
            text: i18nc("@label:textbox", "Display Style")
        }

        ComboBox {
            id: faceCombo

            property string faceId: page.loader.controller.faceId

            model: page.loader.controller.availableFacesModel
            textRole: "display"
            currentIndex: {
                // TODO just make an indexOf invokable on the model?
                for (var i = 0; i < count; ++i) {
                    if (model.pluginId(i) === faceId) {
                        return i;
                    }
                }
                return -1;
            }
            onActivated: (index) => {
                page.loader.controller.faceId = model.pluginId(index);
            }

            Layout.fillWidth: true
        }

        Label {
            Layout.fillWidth: true
            text: i18nc("@label", "Minimum time between updates:")
        }

        SpinBox {
            id: updateRateLimitSpinBox

            Layout.fillWidth: true

            from: 0
            to: 600000
            stepSize: 500
            editable: true

            value: page.loader.controller.updateRateLimit

            textFromValue: function(value, locale) {
                if (value <= 0) {
                    return i18n("No Limit");
                } else {
                    var seconds = value / 1000;
                    if (seconds == 1) { // Manual plural handling because i18ndp doesn't handle floats :(
                        return i18n("1 second");
                    } else {
                        return i18n("%1 seconds", seconds);
                    }
                }
            }
            valueFromText: function(value, locale) {
                // Don't use fromLocaleString here since it will error out on extra
                // characters like the (potentially translated) seconds that gets
                // added above. Instead parseInt ignores non-numeric characters.
                var v = parseInt(value)
                if (isNaN(v)) {
                    return 0;
                } else {
                    return v * 1000;
                }
            }

            onValueModified: page.loader.controller.updateRateLimit = value
        }

        Item {
            id: configContainer

            Layout.fillWidth: true
            implicitHeight: children.length > 0 ? children[0].implicitHeight : 0

            children: loader.controller.faceConfigUi

            Connections {
                target: loader.controller.faceConfigUi

                function onConfigurationChanged() {
                    loader.controller.faceConfigUi.saveConfig()
                    loader.dataObject.markDirty()
                }
            }

            Binding {
                target: configContainer.children[0]
                property: "width"
                value: configContainer.width
            }
        }

        Item {
            id: sensorsContainer

            Layout.fillWidth: true
            implicitHeight: children.length > 0 ? children[0].implicitHeight : 0

            children: loader.controller.sensorsConfigUi

            Connections {
                target: loader.controller.sensorsConfigUi

                function onConfigurationChanged() {
                    loader.controller.sensorsConfigUi.saveConfig()
                    loader.dataObject.markDirty()
                }
            }

            Binding {
                target: sensorsContainer.children[0]
                property: "width"
                value: sensorsContainer.width
            }
        }

        Item { Layout.fillHeight: true; width: 1 }
    }
}
