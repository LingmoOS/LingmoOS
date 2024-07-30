/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.lingmo.core as LingmoCore

MouseArea {
    id: abstractItem
    property string text
    property string itemId
    property int status     //LingmoCore.Types.ItemStatus
    property bool active

    hoverEnabled: true
    drag.filterChildren: true
    acceptedButtons: Qt.RightButton
}

