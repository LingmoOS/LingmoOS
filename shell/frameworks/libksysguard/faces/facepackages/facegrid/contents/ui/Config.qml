/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kitemmodels as KItemModels

import org.kde.ksysguard.sensors as Sensors
import org.kde.ksysguard.faces as Faces

ColumnLayout {
    id: root

    property alias cfg_columnCount: columnCountSpin.value
    property string cfg_faceId

    property var faceController: controller

    // The pluginId role from the FacesModel. FacesModel is a private class that
    // is only created by FaceController, so we can't access its roles from QML.
    // So declare this here since we need to use it in multiple places.
    readonly property int pluginIdRole: Qt.UserRole + 1

    Kirigami.FormLayout {
        id: form

        SpinBox {
            id: columnCountSpin
            Kirigami.FormData.label: i18n("Number of Columns:")
            editable: true
            from: 0
            to: 99999

            textFromValue: function(value, locale) {
                if (value <= 0) {
                    return i18nc("@label", "Automatic")
                }
                return value.toString()
            }

            valueFromText: function(value, locale) {
                return parseInt(value)
            }
        }

        ComboBox {
            id: faceCombo
            Kirigami.FormData.label: i18n("Display Style:")

            model: KItemModels.KSortFilterProxyModel {
                sourceModel: controller.availableFacesModel

                filterRowCallback: function(row, parent) {
                    const pluginId = sourceModel.data(sourceModel.index(row, 0, parent), root.pluginIdRole)
                    const excludedPlugins = [
                        "org.kde.ksysguard.facegrid",
                        "org.kde.ksysguard.colorgrid",
                        "org.kde.ksysguard.applicationstable",
                        "org.kde.ksysguard.processtable"
                    ]

                    return !excludedPlugins.includes(pluginId)
                }

                sortRoleName: "display"
            }

            textRole: "display"
            currentIndex: {
                // TODO just make an indexOf invocable on the model?
                for (var i = 0; i < count; ++i) {
                    const pluginId = model.data(model.index(i, 0), root.pluginIdRole)
                    if (pluginId === root.cfg_faceId) {
                        return i;
                    }
                }
                return -1;
            }
            onActivated: {
                root.cfg_faceId = model.data(model.index(index, 0), root.pluginIdRole)
            }
        }
    }

    Faces.FaceLoader {
        id: loader
        parentController: root.faceController
        groupName: "FaceGrid"
        faceId: root.cfg_faceId
        readOnly: false
    }

    Item {
        Layout.fillWidth: true
        implicitHeight: children.length > 0 ? children[0].implicitHeight : 0

        children: loader.controller ? loader.controller.faceConfigUi : null

        onWidthChanged: {
            if (children.length > 0) {
                children[0].width = width
            }
        }

        Connections {
            target: loader.controller ? loader.controller.faceConfigUi : null

            function onConfigurationChanged() {
                loader.controller.faceConfigUi.saveConfig()
                root.faceController.faceConfigUi.configurationChanged()
            }
        }
    }
}
