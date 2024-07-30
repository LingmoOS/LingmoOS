/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.ksvg 1.0 as KSvg
import org.kde.lingmoui 2.20 as LingmoUI

Item {
    id: iconsDelegate
    Rectangle {
        id: background
        anchors {
            fill: parent
            margins: LingmoUI.Units.gridUnit
        }
        radius: 3
        color: LingmoUI.Theme.backgroundColor
        opacity: 0.6
    }

    property var iconElements: model.iconElements
    property var imagePath: model.imagePath

    Flow {
        clip: true
        anchors {
            fill: background
            margins: LingmoUI.Units.gridUnit
        }
        Repeater {
            model: iconsDelegate.iconElements
            delegate: KSvg.SvgItem {
                imagePath: iconsDelegate.imagePath
                elementId: modelData
                width: naturalSize.width
                height: naturalSize.height
            }
        }
    }

    LingmoComponents.Label {
        anchors {
            horizontalCenter: background.horizontalCenter
            bottom: background.bottom
        }
        text: model.imagePath
        visible: width < background.width
    }
}
