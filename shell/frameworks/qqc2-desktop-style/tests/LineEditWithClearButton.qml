/*
   SPDX-FileCopyrightText: 2019 Montel Laurent <montel@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

import org.kde.lingmoui as LingmoUI

LingmoUI.ActionTextField {
    id: root

    focus: true
    rightActions: LingmoUI.Action {
        iconName: "edit-clear"
        visible: root.text !== ""
        onTriggered: {
            root.text = ""
            root.accepted()
        }
    }
}
