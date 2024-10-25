/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as Controls
import org.kde.lingmoui as LingmoUI
import org.kde.ksvg as KSvg

KSvg.FrameSvgItem {
    id: root
    imagePath: "widgets/background"
    state: KSvg.Svg.Normal
    width: 600
    height: 800

    Column {
        anchors.centerIn: parent
        spacing: 4

        Controls.Button {
            text: "Switch Selected State"
            onClicked: root.state = (root.state == KSvg.Svg.Selected ? KSvg.Svg.Normal : KSvg.Svg.Selected)
        }

        KSvg.SvgItem {
            svg: KSvg.Svg {
                id: svg
                imagePath: "icons/phone"
                state: root.state
            }
        }

        LingmoUI.Icon {
            id: icon
            source: "phone"
            state: root.state
        }
    }
}
