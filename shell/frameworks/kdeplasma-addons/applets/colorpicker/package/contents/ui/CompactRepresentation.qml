/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.kwindowsystem as KWindowSystem
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid
import org.kde.plasma.private.colorpicker as ColorPicker

import "logic.js" as Logic

DropArea {
    id: dropArea

    Layout.minimumWidth: root.isVertical ? Kirigami.Units.iconSizes.small : (grid.width + spacer.implicitWidth)
    Layout.minimumHeight: root.isVertical ? (grid.height + spacer.implicitHeight) : Kirigami.Units.iconSizes.small
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight

    readonly property real buttonSize: root.isVertical ? parent.width : parent.height
    property bool containsAcceptableDrag: false

    onEntered: drag => {
        containsAcceptableDrag = (drag.hasColor || drag.hasUrls || ColorPicker.Utils.isValidColor(drag.text));
    }
    onExited: containsAcceptableDrag = false
    onDropped: drop => {
        if (drop.hasColor) {
            addColorToHistory(drop.colorData)
        } else if (ColorPicker.Utils.isValidColor(drop.text)) {
            addColorToHistory(drop.text)
        } else if (drop.hasUrls) {
            // Show loading indicator above the pick button if there are more than 1 circle
            const indicatorParent = circleRepeater.count === 1 ? circleRepeater.itemAt(0) : pickColorButton;
            Logic.showLoadingIndicator(indicatorParent, drop.urls);
            const component = Qt.createComponent("ImageColors.qml");
            drop.urls.forEach(path => {
                component.incubateObject(indicatorParent, {
                    "source": path,
                    "loadingIndicator": indicatorParent.loadingIndicator,
                }, Qt.Asynchronous);
            });
            component.destroy();
        }
        containsAcceptableDrag = false
    }

    GridLayout {
        id: grid

        width: root.isVertical ? dropArea.buttonSize : implicitWidth
        height: root.isVertical ? implicitHeight : dropArea.buttonSize

        anchors.centerIn: parent
        columns: root.isVertical ? 1 : (1 + (circleRepeater.count > 0 ? circleRepeater.count + 1 : 0))
        rows: root.isVertical ? (1 + (circleRepeater.count > 0 ? circleRepeater.count + 1 : 0)) : 1
        rowSpacing: 0
        columnSpacing: 0

        PlasmaComponents3.ToolButton {
            id: pickColorButton

            Layout.preferredWidth: dropArea.buttonSize
            Layout.preferredHeight: dropArea.buttonSize

            property Item loadingIndicator: null

            display: PlasmaComponents3.AbstractButton.IconOnly
            enabled: KWindowSystem.KWindowSystem.isPlatformWayland || KWindowSystem.KX11Extras.compositingActive
            text: i18nc("@info:tooltip", "Pick color")

            onClicked: root.pickColor()

            PlasmaCore.ToolTipArea {
                anchors.fill: parent
                mainText: parent.text
                subText: xi18nc("@info:usagetip", "Drag a color code here to save it<nl/>Drag an image file here to get its average color");
            }

            Kirigami.Icon {
                id: pickerIcon
                anchors.centerIn: parent
                width: Math.round(parent.width * 0.9)
                height: width
                source: "color-picker"
                active: parent.hovered
            }
        }

        Item { // spacer
            id: spacer

            readonly property real thickness: dropArea.buttonSize / Kirigami.Units.iconSizes.small

            Layout.preferredWidth: root.isVertical ? parent.width : thickness
            Layout.preferredHeight: root.isVertical ? thickness : parent.height
            visible: circleRepeater.count > 0

            Rectangle {
                anchors.centerIn: parent
                width: circleRepeater.count > 0 ? Math.min(parent.width, circleRepeater.itemAt(0).colorCircle.width) : 0
                height: circleRepeater.count > 0 ? Math.min(parent.height, circleRepeater.itemAt(0).colorCircle.height) : 0
                color: Kirigami.Theme.textColor
                opacity: 0.6
            }
        }

        Repeater {
            id: circleRepeater

            model: Plasmoid.configuration.compactPreviewCount

            delegate: ColorCircle {
                Layout.preferredWidth: dropArea.buttonSize
                Layout.preferredHeight: dropArea.buttonSize

                color: historyModel.count > index ? historyModel.get(index).color : "#00000000" // transparent as fallback
            }
        }
    }
}
