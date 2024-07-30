/*
 *  SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.plasmoid 2.0

PlasmoidItem {
    id: root
    property string reason

    fullRepresentation: RowLayout {
        Layout.minimumWidth: LingmoUI.Units.gridUnit * 20
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 8

        clip: true

        LingmoUI.Icon {
            id: icon
            Layout.alignment: Qt.AlignVCenter
            Layout.minimumWidth: LingmoUI.Units.iconSizes.huge
            Layout.minimumHeight: LingmoUI.Units.iconSizes.huge
            source: "dialog-error"
        }

        LingmoComponents.TextArea {
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
