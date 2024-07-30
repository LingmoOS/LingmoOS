/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.plasmoid

import "logic.js" as Logic

PlasmaComponents3.ToolButton {
    id: colorButton

    property alias color: colorCircle.color
    property alias colorCircle: colorCircle
    property Item loadingIndicator: null

    // indicate viable drag...
    checked: (dropArea.containsAcceptableDrag && index === 0) || colorButton.pressed || dragHandler.active
    checkable: checked
    display: PlasmaComponents3.AbstractButton.IconOnly
    text: Logic.formatColor(colorCircle.color, root.defaultFormat)

    Drag.dragType: Drag.Automatic
    Drag.supportedActions: Qt.CopyAction
    Drag.mimeData: {
        "application/x-color": colorCircle.color,
        "text/plain": colorButton.text
    }

    onClicked: root.expanded = !root.expanded

    PlasmaCore.ToolTipArea {
        anchors.fill: parent
        mainText: colorButton.text
        subText: i18nc("@info:usagetip", "Middle-click to copy the color code")
    }

    Rectangle {
        id: colorCircle
        anchors.centerIn: parent
        // try to match the color-picker icon in size
        width: Kirigami.Units.iconSizes.roundedIconSize(pickerIcon.width) * 0.75
        height: Kirigami.Units.iconSizes.roundedIconSize(pickerIcon.height) * 0.75
        radius: width / 2

        function luminance(color) {
            if (!color) {
                return 0;
            }

            // formula for luminance according to https://www.w3.org/TR/2008/REC-WCAG20-20081211/#relativeluminancedef

            const a = [color.r, color.g, color.b].map(v =>
                (v <= 0.03928) ? v / 12.92 : Math.pow(((v + 0.055) / 1.055), 2.4 ));

            return a[0] * 0.2126 + a[1] * 0.7152 + a[2] * 0.0722;
        }

        border {
            color: Kirigami.Theme.textColor
            width: {
                const contrast = luminance(Kirigami.Theme.viewBackgroundColor) / luminance(colorCircle.color) + 0.05;

                // show border only if there's too little contrast to the surrounding view or color is transparent
                if (contrast > 3 && colorCircle.color.a > 0.5) {
                    return 0;
                } else {
                    return Math.round(Math.max(1, width / 20));
                }
            }
        }

        DragHandler {
            id: dragHandler

            enabled: historyModel.count > 0

            onActiveChanged: if (active) {
                colorCircle.grabToImage((result) => {
                    colorButton.Drag.imageSource = result.url;
                    colorButton.Drag.active = dragHandler.active;
                });
            } else {
                colorButton.Drag.active = false;
                colorButton.Drag.imageSource = "";
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: (eventPoint, button) => {
                colorButton.clicked();
            }
        }

        TapHandler {
            acceptedButtons: Qt.MiddleButton
            onTapped: (eventPoint, button) => {
                picker.copyToClipboard(colorButton.text);
            }
        }
    }
}
