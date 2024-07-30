/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Layouts 1.2

import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.ocean.components
import "timer.js" as AutoTriggerTimer

ActionButton {
    Layout.alignment: Qt.AlignTop

    icon.width: LingmoUI.Units.iconSizes.huge
    icon.height: LingmoUI.Units.iconSizes.huge

    font.underline: false // See https://phabricator.kde.org/D9452
    opacity: activeFocus || hovered ? 1 : 0.5

    Keys.onPressed: {
        AutoTriggerTimer.cancelAutoTrigger();
    }
}
