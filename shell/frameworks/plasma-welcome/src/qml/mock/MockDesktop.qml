/*
 *  SPDX-FileCopyrightText: 2024 Oliver Beard <olib141@outlook.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Effects

import org.kde.kirigami as Kirigami
import org.kde.plasma.core as PlasmaCore

import org.kde.plasma.welcome

Item {
    id: root

    property int backgroundAlignment: Qt.AlignRight | Qt.AlignBottom

    readonly property string wallpaper: "file:" + Controller.installPrefix() + "/share/wallpapers/Next/contents/images/1280x800.png"
    readonly property int desktopWidth: 1280
    readonly property int desktopHeight: 800

    /*
     * 1280x800 is chosen to look good at normal window sizes: The aspect closely
     * matches that of the mock as shown in the application and is not too large.
     *
     * The background be positioned according to the alignment, and expand to fit
     * when the mock exceeds its size.
     *
     * Depending on the effect wanted, content can exceed the size to show a region
     * of the desktop, or use the size available.
     */

    clip: true

    // Use Plasma theme colours, rather than Kirigami's
    Kirigami.Theme.inherit: false
    Kirigami.Theme.textColor: PlasmaCore.Theme.textColor
    Kirigami.Theme.backgroundColor: PlasmaCore.Theme.backgroundColor
    Kirigami.Theme.highlightColor: PlasmaCore.Theme.highlightColor

    Image {
        width: Math.max(root.width, root.desktopWidth)
        height: Math.max(root.height, root.desktopHeight)

        anchors.left: (root.backgroundAlignment & Qt.AlignLeft) ? parent.left : undefined
        anchors.right: (root.backgroundAlignment & Qt.AlignRight) ? parent.right: undefined
        anchors.horizontalCenter: (root.backgroundAlignment & Qt.AlignHCenter) ? parent.horizontalCenter : undefined
        anchors.top: (root.backgroundAlignment & Qt.AlignTop) ? parent.top : undefined
        anchors.bottom: (root.backgroundAlignment & Qt.AlignBottom) ? parent.bottom : undefined
        anchors.verticalCenter: (root.backgroundAlignment & Qt.AlignVCenter) ? parent.verticalCenter : undefined

        fillMode: Image.PreserveAspectCrop
        source: root.wallpaper

        layer.enabled: true
        layer.effect: MultiEffect {
            autoPaddingEnabled: false // Prevents blur from lightening at the edges
            blurEnabled: true
            blur: 1.0
            blurMax: 32
        }
    }
}
