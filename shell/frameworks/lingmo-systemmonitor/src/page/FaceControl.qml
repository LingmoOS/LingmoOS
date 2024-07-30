/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.page

Container {
    id: control

    property alias dataObject: loader.dataObject

    function replaceSensors(replacement) {
        let changed = false

        for (let entry of replacement) {
            if (entry.face != dataObject.face) {
                continue
            }

            loader.controller.replaceSensors(entry.sensor, entry.replacement)
            changed = true
        }

        if (changed) {
            loader.controller.reloadConfig()
        }
    }

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    minimumContentHeight: contentItem ? contentItem.Layout.minimumHeight : 0
    minimumHeight: minimumContentHeight + topPadding + bottomPadding

    toolbar.visible: false

    onActiveChanged: {
        if (active) {
            applicationWindow().pageStack.push(Qt.resolvedUrl("FaceConfigurationPage.qml"), {"loader": loader})
        } else {
            if (activeItem instanceof FaceControl) {
                return
            } else {
                if (applicationWindow().pageStack.depth > 1) {
                    applicationWindow().pageStack.pop()
                }
            }
        }
    }

    FaceLoader {
        id: loader
    }

    MouseArea {
        anchors.fill: parent
        z: 99
        onClicked: control.select(control)
    }

    Component.onCompleted: updateContentItem()

    Connections {
        target: loader.controller
        function onFaceIdChanged() {
            control.updateContentItem()
        }

        function onMissingSensorsChanged() {
            control.missingSensorsChanged(dataObject.face, controller.title, loader.controller.missingSensors)
        }
    }

    function updateContentItem() {
        loader.controller.fullRepresentation.formFactor = Faces.SensorFace.Constrained;
        control.contentItem = loader.controller.fullRepresentation;

        if (loader.controller.missingSensors.length > 0) {
            control.missingSensorsChanged(dataObject.face, loader.controller.title, loader.controller.missingSensors)
        }
    }
}
