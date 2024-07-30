/*
    SPDX-FileCopyrightText: 2010 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick 2.1
import org.kde.kirigami 2.20 as Kirigami

MouseArea {
    id: listItem

    default property alias content: paddingItem.data
    property bool checked: false
    property bool sectionDelegate: false

    width: parent ? parent.width : undefined
    height: paddingItem.childrenRect.height + background.margins.top + background.margins.bottom
    implicitHeight: paddingItem.childrenRect.height + Math.round(Kirigami.Units.gridUnit / 2)
    property bool changeBackgroundOnPress: !listItem.checked && !listItem.sectionDelegate
    hoverEnabled: true

    onPressed: if (changeBackgroundOnPress) background.prefix = "pressed"
    onReleased: if (changeBackgroundOnPress) background.prefix = "normal"
    onCanceled: if (changeBackgroundOnPress) background.prefix = "normal"

    Rectangle {
        id: background

        anchors.fill: parent
        visible: listItem.ListView.view ? listItem.ListView.view.highlight === null : true
        opacity: !listItem.checked && listItem.containsMouse && !listItem.pressed ? 0.5 : 1
        color: listItem.sectionDelegate ? alternateBaseColor : (listItem.checked || listItem.containsMouse ? highlightColor : baseColor)
    }

    Item {
        id: paddingItem
        anchors {
            fill: parent
            margins: Math.round(Kirigami.Units.gridUnit / 3)
        }
    }

    Accessible.role: Accessible.ListItem
}
