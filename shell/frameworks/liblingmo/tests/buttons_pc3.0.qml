/*
 * SPDX-FileCopyrightText: 2018 Aleix Pol <aleixpol@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick

import org.kde.lingmo.components as LingmoComponents

Rectangle
{
    width: 500
    height: 300
    color: "white"

    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        LingmoComponents.Button {
            text: "test"
        }
        LingmoComponents.Button {
            text: "test"
            flat: true
        }
        LingmoComponents.ToolButton {
            text: "test"
        }
    }
}

