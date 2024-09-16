// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick 2.15

import org.deepin.ds.dock 1.0
import org.deepin.dtk as D

Item {
    required property int displayMode
    required property int colorTheme
    required property bool active
    required property color backgroundColor

    property color dotIndicator: {
        if (displayMode === Dock.Efficient) {
            if (colorTheme === Dock.Dark) {
                return active ? Qt.rgba(1, 1, 1, 0.6) : Qt.rgba(1, 1, 1, 0.3)
            } else {
                return active ? Qt.rgba(0, 0, 0, 0.8) : Qt.rgba(0, 0, 0, 0.3)
            }
        } else if (displayMode === Dock.Fashion) {
            if (colorTheme === Dock.Dark) {
                return active ? backgroundColor : Qt.rgba(1, 1, 1)
            } else {
                return active ? backgroundColor : Qt.rgba(0, 0, 0)
            }
        } else {
            return "#00000000"
        }
    }

    // Actually only in fashion mode will dot indicator have border
    property color dotIndicatorBorder: {
        return colorTheme === Dock.Dark ? Qt.rgba(0, 0, 0, 0.2) : Qt.rgba(1, 1, 1, 0.2)
    }

    property color rectIndicator: {
        if (colorTheme === Dock.Light) {
            return active ? Qt.rgba(0, 0, 0, 0.8) : Qt.rgba(0, 0, 0, 0.3)
        } else {
            return active ? Qt.rgba(1, 1, 1, 0.6) : Qt.rgba(1, 1, 1, 0.3)
        }
    }

    property color rectIndicatorBorder: {
        return colorTheme === Dock.Light ? Qt.rgba(1, 1, 1, 0.2) : Qt.rgba(0, 0, 0, 0.35)
    }
}
