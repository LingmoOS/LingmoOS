/*
 *  SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.lingmoui as LingmoUI

ColumnLayout {
    width: 600
    height: 600

    Text {
        text: `Implicit width: ${hfLayout.implicitWidth}\nImplicit height: ${hfLayout.implicitHeight}`
    }
    LingmoUI.HeaderFooterLayout {
        id: hfLayout
        Layout.fillWidth: true
        Layout.fillHeight: true
        header: ToolBar {
            contentItem: Rectangle {
                color: "red"
                implicitWidth: 20
                implicitHeight: 20
            }
        }
        contentItem: Rectangle {
            color: "lightgreen"
            implicitWidth: 300
            implicitHeight: 50
        }
        footer: ToolBar {
            contentItem: Rectangle {
                color: "blue"
                height: 30
            }
        }
    }
    Rectangle {
        color: "yellow"
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
}
