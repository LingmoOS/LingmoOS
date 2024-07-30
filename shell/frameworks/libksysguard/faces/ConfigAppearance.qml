/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols
import org.kde.config
import org.kde.newstuff as NewStuff

import org.kde.quickcharts as Charts
import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

Kirigami.FormLayout {
    id: root

    signal configurationChanged

    function saveConfig() {
        controller.title = cfg_title;
        controller.faceId = cfg_chartFace;
        controller.showTitle = cfg_showTitle
        controller.updateRateLimit = cfg_updateRateLimit

        var preset = pendingPreset;
        pendingPreset = "";
        if (preset != "") {
            controller.loadPreset(preset);
            root.controller.highPrioritySensorColors = automaticColorSource.colors
        }
    }

    property Faces.SensorFaceController controller
    property alias cfg_title: titleField.text
    property alias cfg_showTitle: showTitleCheckbox.checked
    property string cfg_chartFace
    property alias cfg_updateRateLimit: updateRateLimitSpinBox.value

    onCfg_titleChanged: configurationChanged();
    onCfg_showTitleChanged: configurationChanged()
    onCfg_chartFaceChanged: configurationChanged();
    onCfg_updateRateLimitChanged: configurationChanged();

    // config keys of the selected preset to be applied on save
    property string pendingPreset

    Component.onCompleted: {
        cfg_title = controller.title;
        cfg_chartFace = controller.faceId;
        cfg_showTitle = controller.showTitle
        cfg_updateRateLimit = controller.updateRateLimit
    }

    Charts.ColorGradientSource {
        id: automaticColorSource
        baseColor: Kirigami.Theme.highlightColor
        itemCount: root.controller.highPrioritySensorIds.length
    }

    Kirigami.OverlaySheet {
        id: presetSheet
        parent: root
        ListView {
            implicitWidth: Kirigami.Units.gridUnit * 15
            focus: true
            model: controller.availablePresetsModel
            delegate: Kirigami.SwipeListItem {
                contentItem: QQC2.Label {
                    elide: Text.ElideRight
                    text: model.display
                }
                actions: Kirigami.Action {
                    icon.name: "delete"
                    visible: model.writable
                    onTriggered: controller.uninstallPreset(model.pluginId);
                }
                onClicked: {
                    cfg_title = model.display;
                    pendingPreset = model.pluginId;
                    if (model.config.chartFace) {
                        cfg_chartFace = model.config.chartFace;
                    }

                    root.configurationChanged();
                    presetSheet.close();
                }

                // shortcut overrides can only be on the item with focus
                Keys.onShortcutOverride: {
                    if (event.key === Qt.Key_Escape) {
                        event.accepted = true;
                        root.close();
                    }
                }
            }
        }
    }
    RowLayout {
        Kirigami.FormData.label: i18nd("KSysGuardSensorFaces", "Presets:")

        QQC2.Button {
            icon.name: "document-open"
            text: i18nd("KSysGuardSensorFaces", "Load Preset...")
            onClicked: presetSheet.open()
        }

        NewStuff.Button {
            Accessible.name: i18nd("KSysGuardSensorFaces", "Get new presets...")
            configFile: "systemmonitor-presets.knsrc"
            text: ""
            onEntryEvent: controller.availablePresetsModel.reload();
            QQC2.ToolTip {
                text: parent.Accessible.name
            }
        }

        QQC2.Button {
            id: saveButton
            icon.name: "document-save"
            text: i18nd("KSysGuardSensorFaces", "Save Settings As Preset")
            onClicked: controller.savePreset();
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    RowLayout {
        Kirigami.FormData.label: i18nd("KSysGuardSensorFaces", "Title:")
        QQC2.TextField {
            id: titleField
        }
        QQC2.CheckBox {
            id: showTitleCheckbox
            text: i18nd("KSysGuardSensorFaces", "Show Title")
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18nd("KSysGuardSensorFaces", "Display Style:")
        QQC2.ComboBox {
            id: faceCombo
            model: controller.availableFacesModel
            textRole: "display"
            currentIndex: {
                // TODO just make an indexOf invocable on the model?
                for (var i = 0; i < count; ++i) {
                    if (model.pluginId(i) === cfg_chartFace) {
                        return i;
                    }
                }
                return -1;
            }
            onActivated: {
                cfg_chartFace = model.pluginId(index);
            }
        }

        NewStuff.Button {
            text: i18nd("KSysGuardSensorFaces", "Get New Display Styles...")
            configFile: "systemmonitor-faces.knsrc"
            onEntryEvent: controller.availableFacesModel.reload();
        }
    }

    QQC2.SpinBox {
        id: updateRateLimitSpinBox
        Layout.preferredWidth: titleField.implicitWidth

        Kirigami.FormData.label: i18nd("KSysGuardSensorFaces", "Minimum Time Between Updates:")

        from: 0
        to: 600000
        stepSize: 500
        editable: true

        textFromValue: function(value, locale) {
            if (value <= 0) {
                return i18nd("KSysGuardSensorFaces", "No Limit");
            } else {
                var seconds = value / 1000;
                if (seconds == 1) { // Manual plural handling because i18ndp doesn't handle floats :(
                    return i18nd("KSysGuardSensorFaces", "1 second");
                } else {
                    return i18nd("KSysGuardSensorFaces", "%1 seconds", seconds);
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
    }
}
