/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0

PlasmoidItem {
    id: root
    property string reason

    fullRepresentation: RowLayout {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 20
        Layout.minimumHeight: Kirigami.Units.gridUnit * 8

        clip: true

        Kirigami.Icon {
            id: icon
            Layout.alignment: Qt.AlignVCenter
            Layout.minimumWidth: Kirigami.Units.iconSizes.huge
            Layout.minimumHeight: Kirigami.Units.iconSizes.huge
            source: "dialog-error"
        }

        PlasmaComponents.TextArea {
            id: messageText
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalAlignment: TextEdit.AlignVCenter
            readOnly: true
            width: parent.width - icon.width
            wrapMode: TextEdit.Wrap
            text: root.reason
        }
    }
}
