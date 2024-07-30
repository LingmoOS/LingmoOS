/*
    SPDX-FileCopyrightText: 2024 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid


ContainmentItem {
    id: root
    width: 640
    height: 48
    readonly property bool verticalPanel: Plasmoid.formFactor === LingmoCore.Types.Vertical

    implicitWidth: verticalPanel ? 32 : 800
    implicitHeight: verticalPanel ? 800 : 32
    Layout.preferredWidth: implicitWidth
    Layout.preferredHeight: implicitHeight
}
