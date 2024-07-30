/*
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.private.kcm_mouse as Mouse

LingmoUI.InlineMessage {
    required property Mouse.message message

    position: LingmoUI.InlineMessage.Position.Header
    visible: message.type !== Mouse.MessageType.None
    text: message.text
    type: {
        switch (message.type) {
        case Mouse.MessageType.Error:
            return LingmoUI.MessageType.Error;
        case Mouse.MessageType.Information:
        case Mouse.MessageType.None:
        default:
            return LingmoUI.MessageType.Information;
        }
    }
}
