/*
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.8

import org.kde.lingmo.extras 2.0 as LingmoExtras

import org.kde.kquickcontrolsaddons 2.0 as KQCAddons

LingmoExtras.Menu {
    id: contextMenu

    signal closed

    property QtObject __clipboard: KQCAddons.Clipboard { }

    // can be a Text or TextEdit
    property Item target

    property string link

    onStatusChanged: {
        if (status === LingmoExtras.Menu.Closed) {
            closed();
        }
    }

    LingmoExtras.MenuItem {
        text: i18ndc("lingmo_applet_org.kde.lingmo.notifications", "@action:inmenu", "Copy Link Address")
        icon: "edit-copy-symbolic"
        onClicked: __clipboard.content = contextMenu.link
        visible: contextMenu.link !== ""
    }

    LingmoExtras.MenuItem {
        separator: true
        visible: contextMenu.link !== ""
    }

    LingmoExtras.MenuItem {
        text: i18ndc("lingmo_applet_org.kde.lingmo.notifications", "@action:inmenu", "Copy")
        icon: "edit-copy-symbolic"
        enabled: typeof target.selectionStart !== "undefined"
        ? target.selectionStart !== target.selectionEnd
        : (target.text || "").length > 0
        onClicked: {
            if (typeof target.copy === "function") {
                target.copy();
            } else {
                __clipboard.content = target.text;
            }
        }
    }

    LingmoExtras.MenuItem {
        id: selectAllAction
        icon: "edit-select-all-symbolic"
        text: i18ndc("lingmo_applet_org.kde.lingmo.notifications", "@action:inmenu", "Select All")
        onClicked: target.selectAll()
        visible: typeof target.selectAll === "function"
    }
}
