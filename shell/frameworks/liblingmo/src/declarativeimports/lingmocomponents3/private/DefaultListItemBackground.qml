/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
//for Settings
import QtQuick.Templates as T
import org.kde.ksvg as KSvg
import org.kde.lingmoui as LingmoUI

KSvg.FrameSvgItem {
    id: background

    required property T.ItemDelegate control

    imagePath: "widgets/listitem"
    prefix: control.highlighted || control.down ? "pressed" : "normal"

    visible: control.ListView.view ? control.ListView.view.highlight === null : true

    KSvg.FrameSvgItem {
        imagePath: "widgets/listitem"
        visible: !LingmoUI.Settings.isMobile
        prefix: "hover"
        anchors.fill: parent
        opacity: background.control.hovered && !background.control.down ? 1 : 0
    }
}
