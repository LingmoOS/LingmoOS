/*
    SPDX-FileCopyrightText: 2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

Item {

    // Initial size of the window in gridUnits
    width: LingmoUI.Units.gridUnit * 28
    height: LingmoUI.Units.gridUnit * 20

    // We use a ColumnLayout to position and size the individual items
    ColumnLayout {

        // Our ColumnLayout is fills the parent item with a bit of margin
        anchors {
            fill: parent
            margins: LingmoUI.Units.gridUnit
        }

        spacing: LingmoUI.Units.gridUnit

        // A title on top
        LingmoUI.Heading {
            level: 1 // from 1 to 5; level 1 is the size used for titles
            text: i18n("Hello Lingmo World!")
        }

        // The central area is a rectangle
        Rectangle {
            // The id is used to reference this item from the
            // button's onClicked function
            id: colorRect

            // It's supposed to grow in both direction
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        // A button to change the color to blue or green
        QQC2.Button {

            // The button is aligned to the right
            Layout.alignment: Qt.AlignRight

            // The button's label, ready for translations
            text: i18n("Change Color")

            onClicked: {
                // Simply switch colors of the rectangle around
                if (colorRect.color != "#b0c4de") {
                    colorRect.color = "#b0c4de"; // lightsteelblue
                } else {
                    colorRect.color = "lightgreen";
                }
                // This message will end up being printed to the terminal
                print("Color is now " + colorRect.color);
            }
        }
    }

    // Overlay everything with a decorative, large, translucent icon
    LingmoUI.Icon {

        // We use an anchor layout and dpi-corrected sizing
        width: LingmoUI.Units.iconSizes.large * 4
        height: width
        anchors {
            left: parent.left
            bottom: parent.bottom
        }

        source: "akregator"
        opacity: 0.1
    }
}
