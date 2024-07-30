/*
    SPDX-FileCopyrightText: 2016 Martin Graesslin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3

LingmoCore.Dialog {
    location: LingmoCore.Types.Floating
    visible: osd.visible
    flags: Qt.FramelessWindowHint
    type: LingmoCore.Dialog.OnScreenDisplay
    outputOnly: true

    mainItem: RowLayout {
        spacing: LingmoUI.Units.smallSpacing
        LingmoUI.Icon {
            implicitWidth: LingmoUI.Units.iconSizes.medium
            implicitHeight: implicitWidth
            source: osd.iconName
            visible: osd.iconName !== ""
        }
        LingmoComponents3.Label {
            text: osd.message
        }
    }
}
