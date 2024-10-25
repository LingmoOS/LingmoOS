/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
import QtQuick

import org.kde.lingmo.plasmoid
import org.kde.lingmo.core as LingmoCore
import org.kde.ksvg as KSvg

KSvg.FrameSvgItem {
    visible: false
    imagePath: {
        if (Window.window instanceof LingmoCore.AppletPopup || Window.window instanceof LingmoCore.Dialog) {
            return "dialogs/background";
        } else if (Plasmoid.formFactor === LingmoCore.Types.Planar) {
            return "widgets/background";
        // panels and other formfactors are explicitly not supported
        } else {
            return "";
        }
    }
    readonly property bool hasInset: inset.left >= 0 && inset.right >= 0 && inset.top >= 0 && inset.bottom >= 0
}
