/*
 * SPDX-FileCopyrightText: 2020 Marco Martin <notmart@gmail.com>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents

ComponentBase {
    id: root
    title: "Lingmo Components 3 TabBar with TabButtons"
    contentItem: LingmoComponents.TabBar {
        implicitWidth: tabButton2.implicitWidth*4
        LingmoComponents.TabButton {
            icon.name: "application-menu"
            text: "Icon Only"
            display: LingmoComponents.TabButton.IconOnly
        }
        LingmoComponents.TabButton {
            id: tabButton2
            icon.name: "application-menu"
            text: "Text Beside Icon"
            display: LingmoComponents.TabButton.TextBesideIcon
        }
        LingmoComponents.TabButton {
            icon.name: "application-menu"
            text: "Text Under Icon"
            display: LingmoComponents.TabButton.TextUnderIcon
        }
        LingmoComponents.TabButton {
            icon.name: "application-menu"
            text: "Text Only"
            display: LingmoComponents.TabButton.TextOnly
        }
    }
}

