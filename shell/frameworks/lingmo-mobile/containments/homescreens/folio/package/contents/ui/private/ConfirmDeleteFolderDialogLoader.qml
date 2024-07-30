// SPDX-FileCopyrightText: 2024 Devin Lin <devin@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Window
import QtQuick.Layouts

import org.kde.lingmo.components 3.0 as PC3
import org.kde.lingmoui as LingmoUI

Loader {
    id: root
    active: false

    // we expect the parent to be set to the homescreen, otherwise the dialog opens with no size
    anchors.fill: parent

    signal accepted()

    function open() {
        root.active = true;
        root.item.open();
    }

    function close() {
        if (root.item) {
            root.item.close();
        }
    }

    sourceComponent: LingmoUI.PromptDialog {
        id: menu
        title: i18n('Confirm Folder Deletion')
        subtitle: i18n('Are you sure you want to delete this folder?')
        standardButtons: LingmoUI.Dialog.Yes | LingmoUI.Dialog.Cancel

        onAccepted: root.accepted()
        onClosed: root.active = false
    }
}
