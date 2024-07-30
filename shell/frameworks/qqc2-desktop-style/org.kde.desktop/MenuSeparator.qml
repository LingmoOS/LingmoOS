/*
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2019 Alexander Stippich <a.stippich@gmx.net>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.MenuSeparator {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: visible ? Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding) : 0

    // Let optional chaining operator fallback to undefined which would call a
    // RESET method so that width would follow implicit width automatically.
    width: parent?.width

    verticalPadding: Math.round(LingmoUI.Units.smallSpacing / 2)
    hoverEnabled: false
    focusPolicy: Qt.NoFocus

    contentItem: LingmoUI.Separator {
        // same as MenuItem background
        implicitWidth: LingmoUI.Units.gridUnit * 8
    }
}
