// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmoui 2.9 as LingmoUI

import PicoWizard 1.0

Item {
    WelcomeModule {
        id: welcomeModule
    }

    Label {
        id: labelWelcome
        font.weight: Font.Light
        font.pointSize: 32
        text: qsTr("Welcome")
        anchors.centerIn: parent
        color: LingmoUI.Theme.textColor
    }

    NextButton {
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: 12
        }
    }
}
