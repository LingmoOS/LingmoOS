/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


import org.kde.kirigami as Kirigami
import org.kde.ksysguard.faces as Faces
import org.kde.ksysguard.page

ColumnLayout {
    id: root

    property PageDataObject pageData
    property var actionsFace
    property var missingSensors: []

    property var faceMapping: new Object()

    signal showMissingSensors()

    function replaceSensors(replacement) {
        if (!replacement) {
            return
        }

        missingSensors = []

        let modifiedControllers = []

        for (let entry of replacement) {
            let controller = faceMapping[entry.face].controller
            controller.replaceSensors(entry.sensor,  entry.replacement)
            modifiedControllers.push(controller)
        }

        for (let c of modifiedControllers) {
            c.reloadConfig()
        }
    }

    spacing: rowSpacing // From parent Loader

    readonly property real balancedRowHeight: {
        let reservedSpace = 0;
        let minimumSpace = 0;
        let balancedCount = 0;
        let maximumCount = 0;

        for (let i in children) {
            let child = children[i]
            if (!child.hasOwnProperty("heightMode") || !child.hasOwnProperty("minimumContentHeight")) {
                continue
            }

            switch(child.heightMode) {
                case "minimum":
                    reservedSpace += child.minimumContentHeight
                    break;
                case "balanced":
                    minimumSpace = Math.max(child.minimumContentHeight, minimumSpace)
                    balancedCount += 1
                    break;
                case "maximum":
                    maximumCount += 1
                    break;
            }
        }

        // If there's nothing to balance, we can ignore the rest.
        if (balancedCount == 0) {
            return 0
        }

        // If there's any rows that are set to "maximum", use the largest
        // minimum size for balanced rows.
        if (maximumCount > 0) {
            return minimumSpace
        }

        // Note that "availableHeight" here comes from the parent loader and
        // represents the available size of the content area of the page.
        let balancedHeight = (availableHeight - reservedSpace - root.spacing * (children.length - 1)) / balancedCount
        return Math.max(balancedHeight, minimumSpace)
    }

    Kirigami.InlineMessage {
        id: missingMessage

        Layout.fillWidth: true

        visible: root.missingSensors.length > 0
        type: Kirigami.MessageType.Error
        text: i18n("This page is missing some sensors and will not display correctly.");

        actions: Kirigami.Action {
            icon.name: "document-edit"
            text: i18nc("@action:button", "Fix…")
            onTriggered: root.showMissingSensors()
        }
    }

    Repeater {
        model: PageDataModel { data: root.pageData }

        ConditionalLoader {
            required property var model

            readonly property real minimumContentHeight: model.data.isTitle ? item.height : item.Layout.minimumHeight

            readonly property string heightMode: {
                if (model.data.heightMode) {
                    return model.data.heightMode
                }

                if (model.data.isTitle) {
                    return "minimum"
                }

                return "balanced"
            }

            Layout.fillWidth: true
            Layout.fillHeight: heightMode != "minimum"
            Layout.preferredHeight: 0
            Layout.minimumHeight: {
                if (heightMode == "minimum") {
                    return minimumContentHeight
                }

                if (heightMode == "balanced") {
                    return root.balancedRowHeight
                }

                return -1
            }
            Layout.maximumHeight: {
                if (heightMode == "minimum") {
                    return minimumContentHeight
                }

                if (heightMode == "balanced") {
                    return root.balancedRowHeight
                }

                return -1
            }

            condition: model.data.isTitle

            sourceTrue: Kirigami.Heading {
                anchors.left: parent.left
                level: 2
                text: model.data.title ? model.data.title : ""
            }

            sourceFalse: GridLayout {
                id: rowContents

                anchors.fill: parent

                columnSpacing: Kirigami.Units.largeSpacing
                rowSpacing: Kirigami.Units.largeSpacing

                Repeater {
                    model: PageDataModel { data: model.data }

                    ConditionalLoader {
                        required property var model

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.minimumHeight: item.Layout.minimumHeight + (model.data.noMargins ? 0 : Kirigami.Units.largeSpacing * 2)
                        Layout.preferredWidth: 0

                        component ColumnContents: RowLayout {
                            id: columnContents

                            anchors.fill: parent
                            anchors.margins: model.data.noMargins ? 0 : Kirigami.Units.largeSpacing

                            spacing: Kirigami.Units.largeSpacing

                            layer.enabled: (model.data.showBackground && model.data.noMargins) ?? false
                            layer.effect: Kirigami.ShadowedTexture {
                                radius: Kirigami.Units.smallSpacing
                            }

                            Repeater {
                                model: PageDataModel { data: model.data }

                                ConditionalLoader {
                                    required property var model

                                    Layout.fillHeight: true
                                    Layout.fillWidth: item?.Layout.fillWidth ?? true
                                    Layout.preferredWidth: item?.Layout.preferredWidth ?? 0
                                    Layout.minimumHeight: item?.Layout.minimumHeight ?? 0

                                    condition: model.data.isSeparator

                                    sourceTrue: Kirigami.Separator { Layout.preferredWidth: 1 }

                                    sourceFalse: faceComponent
                                }
                            }
                        }

                        condition: model.data.showBackground

                        sourceTrue: Kirigami.AbstractCard {
                            anchors.fill: parent

                            Layout.minimumHeight: contents.Layout.minimumHeight

                            ColumnContents { id: contents }
                        }

                        sourceFalse: ColumnContents { }
                    }
                }
            }
        }
    }

    Component {
        id: faceComponent

        Control {
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0

            Layout.preferredWidth: 0
            Layout.minimumHeight: contentItem ? contentItem.Layout.minimumHeight : 0
            Layout.fillWidth: true

            FaceLoader {
                id: loader
                dataObject: model.data

                Component.onCompleted: {
                    root.faceMapping[model.data.face] = loader
                }
            }

            Connections {
                target: loader.controller

                function onMissingSensorsChanged() {
                    for (let i of loader.controller.missingSensors) {
                        root.missingSensors.push({
                            "face": model.data.face,
                            "title": loader.controller.title,
                            "sensor": i
                        })
                    }
                    root.missingSensorsChanged()
                }

                Component.onCompleted: {
                    root.faceMapping[model.data.face] = loader
                }
            }

            Component.onCompleted: {
                loader.controller.fullRepresentation.formFactor = Faces.SensorFace.Constrained;
                contentItem = loader.controller.fullRepresentation

                if (root.pageData.actionsFace == model.data.face) {
                    root.actionsFace = loader.controller.fullRepresentation
                }
            }
            TapHandler {
                acceptedButtons: Qt.RightButton
                enabled: WidgetExporter.plasmashellAvailable
                onTapped: eventPoint => {
                    const point = parent.mapToItem(contextMenu.parent, eventPoint.position)
                    contextMenu.x = point.x
                    contextMenu.y = point.y
                    contextMenu.loader = loader
                    contextMenu.open()
                }
            }
        }
    }
    Menu {
        id: contextMenu
        property QtObject loader
        MenuItem {
            icon.name: "document-export"
            text: i18nc("@action", "Add Chart as Desktop Widget")
            onTriggered: {
                WidgetExporter.exportAsWidget(contextMenu.loader)
            }
        }
    }
}
