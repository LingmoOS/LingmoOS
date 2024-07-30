/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.pipewire 0.1 as PipeWire

Kirigami.Card {
    id: card

    property alias nodeId: pipeWireSourceItem.nodeId

    showClickFeedback: true

    contentItem: PipeWire.PipeWireSourceItem {
        id: pipeWireSourceItem
        Layout.preferredHeight: Kirigami.Units.gridUnit * 7
        Layout.preferredWidth: Kirigami.Units.gridUnit * 7
        Layout.fillWidth: true
        Layout.fillHeight: true

        Kirigami.Icon {
            anchors.fill: parent
            visible: pipeWireSourceItem.nodeId === 0
            source: card.banner.titleIcon
        }
    }

    Layout.preferredHeight: contentItem.Layout.preferredHeight
}
