/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later

    Adapted from KScreen
*/

import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

import org.kde.lingmoui as LingmoUI

Item {
    id: output

    property bool isSelected: true

    onIsSelectedChanged: {
        if (isSelected) {
            z = 89;
        } else {
            z = 0;
        }
    }

    Rectangle {
        id: outline
        radius: LingmoUI.Units.cornerRadius
        color: LingmoUI.Theme.backgroundColor

        anchors.fill: parent

        border {
            color: isSelected ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.disabledTextColor
            width: 1

            ColorAnimation on color {
                duration: LingmoUI.Units.longDuration
            }
        }
    }

    Item {
        id: orientationPanelContainer

        anchors.fill: output
        visible: false

        Rectangle {
            id: orientationPanel
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            height: LingmoUI.Units.largeSpacing
            color: isSelected ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.disabledTextColor
            smooth: true

            ColorAnimation on color {
                duration: LingmoUI.Units.longDuration
            }
        }
    }

    OpacityMask {
        anchors.fill: orientationPanelContainer
        source: orientationPanelContainer
        maskSource: outline
    }
}

