/*
    SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


import QtQuick
import QtQuick.Layouts

import org.kde.ksvg as KSvg

Item
{
    width: 500
    height: 500



    Grid {
        anchors.fill: parent
        columns: 3

        Repeater {
            model: ["widgets/background",
                        "widgets/panel-background",
                        "opaque/widgets/panel-background",
                        "widgets/tooltip",
                        "opaque/widgets/tooltip"
                        ]

            delegate: KSvg.FrameSvgItem {
                width: 100
                height: 100
                imagePath: modelData
            }
        }
    }
}

