/*
 *  SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

ColumnLayout {
    id: root

    property string title

    spacing: 0
    Layout.fillWidth: true

    LingmoUI.Heading {
        Layout.fillWidth: true
        level: 3
        text: root.title
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }

    LingmoUI.Separator {
        Layout.fillWidth: true
        Layout.topMargin: LingmoUI.Units.smallSpacing
        Layout.bottomMargin: LingmoUI.Units.largeSpacing
    }
}
