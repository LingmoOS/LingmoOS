// SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as QQCT
import org.kde.lingmoui 2.19 as LingmoUI

QQCT.Button {
     id: menuBtn
     width: listView.width
     height: LingmoUI.Units.gridUnit * 3
     property alias iconSource: menuBtnIcon.source

     readonly property Flickable listView: {
         var candidate = parent;
         while (candidate) {
             if (candidate instanceof Flickable) {
                 return candidate;
             }
             candidate = candidate.parent;
         }
         return null;
     }

     background: Rectangle {
         color: menuBtn.activeFocus ? LingmoUI.Theme.highlightColor : LingmoUI.Theme.backgroundColor
         radius: 6
     }

     contentItem: RowLayout {
         LingmoUI.Icon {
             id: menuBtnIcon
             Layout.preferredWidth: LingmoUI.Units.iconSizes.medium
             Layout.preferredHeight: LingmoUI.Units.iconSizes.medium
         }

         QQCT.Label {
             id: menuBtnText
             text: menuBtn.text
             color: LingmoUI.Theme.textColor
             wrapMode: Text.WordWrap
             Layout.fillWidth: true
         }
     }

     Keys.onReturnPressed: (event)=> {
         clicked()
     }
 }
