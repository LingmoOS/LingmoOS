/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

var iconSizes = [LingmoUI.Units.iconSizes.smallMedium,
                 LingmoUI.Units.iconSizes.medium,
                 LingmoUI.Units.iconSizes.large,
                 LingmoUI.Units.iconSizes.huge,
                 LingmoUI.Units.iconSizes.large*2,
                 LingmoUI.Units.iconSizes.enormous,
                 LingmoUI.Units.iconSizes.enormous*2];

function iconSizeFromTheme(size) {
    return iconSizes[size];
}

function effectiveNavDirection(flow, layoutDirection, direction) {
    if (direction == Qt.LeftArrow) {
        if (flow == GridView.FlowLeftToRight) {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.LeftArrow;
            } else {
                return Qt.RightArrow;
            }
        } else {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.UpArrow;
            } else {
                return Qt.DownArrow;
            }
        }
    } else if (direction == Qt.RightArrow) {
        if (flow == GridView.FlowLeftToRight) {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.RightArrow;
            } else {
                return Qt.LeftArrow;
            }
        } else {
            if (layoutDirection == Qt.LeftToRight) {
                return Qt.DownArrow;
            } else {
                return Qt.UpArrow;
            }
        }
    } else if (direction == Qt.UpArrow) {
        if (flow == GridView.FlowLeftToRight) {
            return Qt.UpArrow;
        } else {
            return Qt.LeftArrow;
        }
    } else if (direction == Qt.DownArrow) {
        if (flow == GridView.FlowLeftToRight) {
            return Qt.DownArrow;
        } else {
            return Qt.RightArrow
        }
    }
}

function isFileDrag(event) {
    var taskUrl = event.mimeData.formats.indexOf("text/x-orgkdelingmotaskmanager_taskurl") != -1;
    var arkService = event.mimeData.formats.indexOf("application/x-kde-ark-dndextract-service") != -1;
    var arkPath = event.mimeData.formats.indexOf("application/x-kde-ark-dndextract-path") != -1;

    return (event.mimeData.hasUrls || taskUrl || (arkService && arkPath));
}
