// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as QQCT
import org.kde.lingmoui 2.15 as LingmoUI

QQCT.Button {
     id: osdBtn
     Layout.fillWidth: true
     Layout.preferredHeight: LingmoUI.Units.gridUnit * 4
     property alias iconSource: osdBtnIcon.source

     background: Rectangle {
         id: osdBtnBackground
         color: osdBtn.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
         border.color: LingmoUI.Theme.disabledTextColor
     }

     onFocusChanged: {
        hideTimer.restart()
     }

     contentItem: Item {
         LingmoUI.Icon {
             id: osdBtnIcon
             anchors.centerIn: parent
             Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
             Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
             color: osdBtn.checked ? LingmoUI.Theme.negativeTextColor : LingmoUI.Theme.textColor
         }
     }

     Keys.onReturnPressed: (event)=> {
         clicked()
     }
 }
