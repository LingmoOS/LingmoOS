/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import "." as Impl

Rectangle {
    id: root
    property T.ItemDelegate control: root.parent

    readonly property bool highlight: control.highlighted || control.down
    readonly property bool useAlternatingColors: {
        if (control.TableView.view?.alternatingRows && row % 2) {
            return true
        } else if (control.LingmoUI.Theme.useAlternateBackgroundColor && index % 2) {
            return true
        }
        return false
    }

    readonly property color hoverColor: Qt.alpha(LingmoUI.Theme.hoverColor, 0.3)
    readonly property color highlightColor: LingmoUI.Theme.highlightColor
    readonly property color normalColor: useAlternatingColors ? LingmoUI.Theme.alternateBackgroundColor : LingmoUI.Theme.backgroundColor
    // Workaround for QTBUG-113304
    readonly property bool reallyFocus: control.visualFocus || (control.activeFocus && control.focusReason === Qt.OtherFocusReason)

    property real horizontalPadding: LingmoUI.Units.smallSpacing
    property real verticalPadding: LingmoUI.Units.smallSpacing
    property real cornerRadius: Impl.Units.smallRadius

    color: normalColor

    Rectangle {
        anchors {
            fill: parent
            leftMargin: background.horizontalPadding
            rightMargin: background.horizontalPadding
            // We want total spacing between consecutive list items to be
            // verticalPadding. So use half that as top/bottom margin, separately
            // ceiling/flooring them so that the total spacing is preserved.
            topMargin: Math.ceil(root.verticalPadding / 2)
            bottomMargin: Math.floor(root.verticalPadding / 2)
        }

        radius: root.cornerRadius

        color: {
            if (root.highlight) {
                return root.highlightColor
            } else {
                return (root.control.hovered || root.reallyFocus) ? root.hoverColor : root.normalColor
            }
        }

        border.width: 1
        border.color: {
            if (root.highlight) {
                return root.highlightColor
            } else {
                return (root.control.hovered || root.reallyFocus) ? LingmoUI.Theme.hoverColor : "transparent"
            }
        }
    }
}

