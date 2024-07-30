/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Window 2.2

import org.kde.kquickcontrolsaddons 2.0
import org.kde.lingmoui 2.5 as LingmoUI

QQC2.ItemDelegate {
    id: delegate

    signal activated()

//BEGIN properties
    Layout.fillWidth: true
    hoverEnabled: true

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.name
    Accessible.description: i18nd("lingmo_shell_org.kde.lingmo.desktop", "Open configuration page")
    Accessible.onPressAction: delegate.clicked()

    property var item
//END properties

//BEGIN connections
    onClicked: {
        if (highlighted) {
            return;
        }

        activated()
    }
//END connections

//BEGIN UI components
    contentItem: ColumnLayout {
        id: delegateContents
        spacing: LingmoUI.Units.smallSpacing

        LingmoUI.Icon {
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: LingmoUI.Units.iconSizes.medium
            implicitHeight: LingmoUI.Units.iconSizes.medium
            source: model.icon
            selected: Window.active && (delegate.highlighted || delegate.pressed)
        }

        QQC2.Label {
            id: nameLabel
            Layout.fillWidth: true
            Layout.leftMargin: LingmoUI.Units.smallSpacing
            Layout.rightMargin: LingmoUI.Units.smallSpacing
            text: model.name
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            color: Window.active && (delegate.highlighted || delegate.pressed) ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
            font.bold: delegate.highlighted && delegate.parent.activeFocus
            Accessible.ignored: true
        }
    }
//END UI components
}
