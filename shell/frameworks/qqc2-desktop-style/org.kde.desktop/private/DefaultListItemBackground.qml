/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami

Rectangle {
    id: background

    property T.ItemDelegate control

    readonly property bool highlight: control.highlighted || control.down
    readonly property bool useAlternatingColors: {
        if (control.TableView.view?.alternatingRows && row % 2) {
            return true
        } else if (control.Kirigami.Theme.useAlternateBackgroundColor && index % 2) {
            return true
        }
        return false
    }

    readonly property color hoverColor: Qt.alpha(Kirigami.Theme.hoverColor, 0.3)
    readonly property color highlightColor: Kirigami.Theme.highlightColor
    readonly property color normalColor: useAlternatingColors ? Kirigami.Theme.alternateBackgroundColor : "transparent"
    // Workaround for QTBUG-113304
    readonly property bool reallyFocus: control.visualFocus || (control.activeFocus && control.focusReason === Qt.OtherFocusReason)

    readonly property bool hasInset: control.leftInset > 0 || control.rightInset > 0 || control.topInset > 0 || control.bottomInset > 0

    color: normalColor

    Rectangle {
        anchors {
            fill: parent
            leftMargin: background.control.leftInset
            rightMargin: background.control.rightInset
            topMargin: background.control.topInset
            bottomMargin: background.control.bottomInset
        }

        radius: background.hasInset ? Kirigami.Units.cornerRadius : 0

        color: {
            if (background.highlight) {
                return background.highlightColor
            } else {
                return (background.control.hovered || background.reallyFocus) ? background.hoverColor : background.normalColor
            }
        }

        border.width: background.hasInset ? 1 : 0
        border.color: {
            if (background.highlight) {
                return background.highlightColor
            } else {
                return (background.control.hovered || background.reallyFocus) ? Kirigami.Theme.hoverColor : "transparent"
            }
        }
    }
}
