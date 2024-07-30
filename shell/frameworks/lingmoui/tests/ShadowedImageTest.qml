/*
 *  SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    width: 600
    height: 800
    visible: true

    pageStack.initialPage: LingmoUI.Page {
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        Column {
            anchors.centerIn: parent

            LingmoUI.ShadowedImage {
                width: 400
                height: 300

                color: LingmoUI.Theme.highlightColor

                source: "/usr/share/wallpapers/Next/contents/images/1024x768.jpg"

                radius: radiusSlider.value

                shadow.size: sizeSlider.value
                shadow.xOffset: xOffsetSlider.value
                shadow.yOffset: yOffsetSlider.value

                border.width: borderWidthSlider.value
                border.color: LingmoUI.Theme.textColor

                corners.topLeftRadius: topLeftSlider.value
                corners.topRightRadius: topRightSlider.value
                corners.bottomLeftRadius: bottomLeftSlider.value
                corners.bottomRightRadius: bottomRightSlider.value
            }

            LingmoUI.FormLayout {
                Item { LingmoUI.FormData.isSection: true }

                Slider { id: radiusSlider; from: 0; to: 200; LingmoUI.FormData.label: "Overall Radius" }
                Slider { id: topLeftSlider; from: -1; to: 200; value: -1; LingmoUI.FormData.label: "Top Left Radius" }
                Slider { id: topRightSlider; from: -1; to: 200; value: -1; LingmoUI.FormData.label: "Top Right Radius" }
                Slider { id: bottomLeftSlider; from: -1; to: 200; value: -1; LingmoUI.FormData.label: "Bottom Left Radius" }
                Slider { id: bottomRightSlider; from: -1; to: 200; value: -1; LingmoUI.FormData.label: "Bottom Right Radius" }

                Slider { id: sizeSlider; from: 0; to: 100; LingmoUI.FormData.label: "Shadow Size" }
                Slider { id: xOffsetSlider; from: -100; to: 100; LingmoUI.FormData.label: "Shadow X-Offset" }
                Slider { id: yOffsetSlider; from: -100; to: 100; LingmoUI.FormData.label: "Shadow Y-Offset" }

                Slider { id: borderWidthSlider; from: 0; to: 50; LingmoUI.FormData.label: "Border Width" }
            }
        }
    }
}
