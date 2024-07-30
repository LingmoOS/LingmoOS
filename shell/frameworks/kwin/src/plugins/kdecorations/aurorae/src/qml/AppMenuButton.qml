/*
    SPDX-FileCopyrightText: 2012 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.kwin.decoration

DecorationButton {
    id: appMenuButton
    buttonType: DecorationOptions.DecorationButtonApplicationMenu
    visible: decoration.client.hasApplicationMenu
    LingmoUI.Icon {
        anchors.fill: parent
        source: decoration.client.icon
    }
}
