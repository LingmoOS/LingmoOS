/*
 *   SPDX-FileCopyrightText: 2021 Devin Lin <devin@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15

import org.kde.lingmoui 2.20 as LingmoUI

Rectangle {
    id: handle

    signal tapped()

    implicitWidth: LingmoUI.Units.gridUnit * 3
    implicitHeight: 3
    radius: height
    color: LingmoUI.Theme.textColor
    opacity: 0.5

    TapHandler {
        cursorShape: pressed ? Qt.ClosedHandCursor : Qt.PointingHandCursor
        onTapped: handle.tapped()
    }
}
