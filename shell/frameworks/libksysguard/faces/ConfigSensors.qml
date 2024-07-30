/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQml.Models

import Qt.labs.platform as Platform

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols

import org.kde.kitemmodels as KItemModels

import org.kde.quickcharts as Charts
import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

import "./" as Local

ColumnLayout {
    id: root

    signal configurationChanged

    property var cfg_totalSensors: []
    property var cfg_highPrioritySensorIds: []
    property var cfg_sensorColors: new Object()
    property var cfg_lowPrioritySensorIds: []
    property var cfg_sensorLabels: new Object()

    onCfg_totalSensorsChanged: configurationChanged();
    onCfg_highPrioritySensorIdsChanged: configurationChanged();
    onCfg_sensorColorsChanged: configurationChanged();
    onCfg_lowPrioritySensorIdsChanged: configurationChanged();
    onCfg_sensorLabelsChanged: configurationChanged();

    property Faces.SensorFaceController controller

    // In QML someArray = someOtherArray will always trigger a changed signal
    // even if the two arrays are the same
    // to avoid that we implement an explicit check
    function arrayCompare(array1, array2) {
      if (array1.length !== array2.length) {
          return false;
      }
      return array1.every(function(value, index) { return value === array2[index]});
  }

  function saveConfig() {
        controller.totalSensors = cfg_totalSensors;
        controller.highPrioritySensorIds = cfg_highPrioritySensorIds;
        controller.sensorColors = cfg_sensorColors;
        controller.lowPrioritySensorIds = cfg_lowPrioritySensorIds;
        controller.sensorLabels = cfg_sensorLabels;
    }

    function loadConfig() {
        if (!arrayCompare(cfg_totalSensors, controller.totalSensors)) {
            cfg_totalSensors = controller.totalSensors;
            totalChoice.selected = controller.totalSensors;
        }

        if (!arrayCompare(cfg_highPrioritySensorIds, controller.highPrioritySensorIds)) {
            cfg_highPrioritySensorIds = controller.highPrioritySensorIds;
            highPriorityChoice.selected = controller.highPrioritySensorIds;
        }

        if(JSON.stringify(cfg_sensorColors) != JSON.stringify(controller.sensorColors)) {
            cfg_sensorColors = controller.sensorColors;
        }

        if (!arrayCompare(cfg_lowPrioritySensorIds, controller.lowPrioritySensorIds)) {
            cfg_lowPrioritySensorIds = controller.lowPrioritySensorIds;
            lowPriorityChoice.selected = controller.lowPrioritySensorIds;
        }
        if(JSON.stringify(cfg_sensorLabels) != JSON.stringify(controller.sensorLabels)) {
            cfg_sensorLabels = controller.sensorLabels;
        }
    }

    // When the ui is open in systemsettings and the page is switched around,
    // it gets reparented to null. use this to reload its config every time the
    // page is current again. So any non saved change to the sensor list gets forgotten.
    onParentChanged: {
        if (parent) {
            loadConfig()
        }
    }

    Component.onCompleted: loadConfig()

    Connections {
        target: controller
        function onTotalSensorsChanged() {
            Qt.callLater(root.loadConfig);
        }
        function onHighPrioritySensorIdsChanged() {
            Qt.callLater(root.loadConfig);
        }
        function onSensorColorsChanged() {
            Qt.callLater(root.loadConfig);
        }
        function onLowPrioritySensorIdsChanged() {
            Qt.callLater(root.loadConfig);
        }
        function onSensorLabelsChanged() {
            Qt.callLater(root.loadConfig);
        }
    }

    Platform.ColorDialog {
        id: colorDialog
        property string destinationSensor

        currentColor: destinationSensor != "" ? controller.sensorColors[destinationSensor] : ""
        onAccepted: {
            cfg_sensorColors[destinationSensor] = Qt.rgba(color.r, color.g, color.b, color.a);
            root.cfg_sensorColorsChanged();
        }
    }


    QQC2.Label {
        text: i18ndp("KSysGuardSensorFaces", "Total Sensor", "Total Sensors", controller.maxTotalSensors)
        visible: controller.supportsTotalSensors
    }
    Faces.Choices {
        id: totalChoice
        Layout.fillWidth: true
        visible: controller.supportsTotalSensors
        supportsColors: false
        maxAllowedSensors: controller.maxTotalSensors
        labels: root.cfg_sensorLabels

        onSelectedChanged: root.cfg_totalSensors = selected
        onSensorLabelChanged: (sensorId, label) => {
            cfg_sensorLabels[sensorId] = label
            root.cfg_sensorLabelsChanged()
        }
    }

    QQC2.Label {
        text: i18nd("KSysGuardSensorFaces", "Sensors")
    }
    Faces.Choices {
        id: highPriorityChoice
        Layout.fillWidth: true
        supportsColors: controller.supportsSensorsColors
        labels: root.cfg_sensorLabels

        onSelectedChanged: root.cfg_highPrioritySensorIds = selected

        colors: root.cfg_sensorColors
        onSelectColor: sensorId => {
            colorDialog.destinationSensor = sensorId
            colorDialog.open()
        }
        onColorForSensorGenerated: (sensorId, color) => {
            cfg_sensorColors[sensorId] = color
            root.cfg_sensorColorsChanged();
        }
        onSensorLabelChanged: (sensorId, label) => {
            cfg_sensorLabels[sensorId] = label
            root.cfg_sensorLabelsChanged()
        }
    }

    QQC2.Label {
        text: i18nd("KSysGuardSensorFaces", "Text-Only Sensors")
        visible: controller.supportsLowPrioritySensors
    }
    Faces.Choices {
        id: lowPriorityChoice
        Layout.fillWidth: true
        visible: controller.supportsLowPrioritySensors
        supportsColors: false
        labels: root.cfg_sensorLabels

        onSelectedChanged: root.cfg_lowPrioritySensorIds = selected
        onSensorLabelChanged: (sensorId, label) => {
            cfg_sensorLabels[sensorId] = label
            root.cfg_sensorLabelsChanged()
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
