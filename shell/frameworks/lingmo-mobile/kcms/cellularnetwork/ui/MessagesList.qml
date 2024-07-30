// SPDX-FileCopyrightText: 2021 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.12
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.12 as Controls

import org.kde.lingmoui 2.12 as LingmoUI

import cellularnetworkkcm 1.0

ColumnLayout {
    id: root

    property var model
    property alias count: repeater.count

    spacing: 0
    visible: count > 0

    Repeater {
        id: repeater
        model: root.model

        delegate: LingmoUI.InlineMessage {
            Layout.bottomMargin: LingmoUI.Units.largeSpacing
            Layout.fillWidth: true
            visible: true
            text: modelData.message
            type: {
                switch (modelData.type) {
                    case InlineMessage.Information: return LingmoUI.MessageType.Information;
                    case InlineMessage.Positive: return LingmoUI.MessageType.Positive;
                    case InlineMessage.Warning: return LingmoUI.MessageType.Warning;
                    case InlineMessage.Error: return LingmoUI.MessageType.Error;
                }
                return LingmoUI.MessageType.Error;
            }

            actions: [
                LingmoUI.Action {
                    icon.name: "dialog-close"
                    onTriggered: kcm.removeMessage(model.index)
                }
            ]
        }
    }
}
