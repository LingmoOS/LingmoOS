/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15
import QtQuick.Dialogs
import org.kde.lingmoui 2.20 as LingmoUI

Button {
    id: colorButton
    Layout.minimumWidth: LingmoUI.Units.gridUnit * 3
    onClicked: {
        colorDialog.activeButton = colorButton;
        colorDialog.open();
    }
    property alias color: colorRect.color;

    Rectangle {
        id: colorRect
        anchors.centerIn: parent
        width: parent.width - 2 * LingmoUI.Units.smallSpacing
        height: LingmoUI.Units.gridUnit
    }
}
