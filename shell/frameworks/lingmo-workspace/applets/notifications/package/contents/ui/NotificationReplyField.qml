/*
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.8
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.20 as LingmoUI

RowLayout {
    id: replyRow

    signal beginReplyRequested
    signal replied(string text)

    property bool replying: false

    property alias text: replyTextField.text
    property string placeholderText
    property string buttonIconName
    property string buttonText

    spacing: LingmoUI.Units.smallSpacing

    function activate() {
        replyTextField.forceActiveFocus();
    }

    LingmoComponents3.TextField {
        id: replyTextField
        Layout.fillWidth: true
        placeholderText: replyRow.placeholderText
                         || i18ndc("lingmo_applet_org.kde.lingmo.notifications", "Text field placeholder", "Type a reply…")
        onAccepted: {
            if (replyButton.enabled) {
                replyRow.replied(text);
            }
        }

        // Catches mouse click when reply field is already shown to start a reply
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.IBeamCursor
            visible: !replyRow.replying
            onPressed: replyRow.beginReplyRequested()
        }
    }

    LingmoComponents3.Button {
        id: replyButton
        icon.name: replyRow.buttonIconName || "document-send"
        text: replyRow.buttonText
              || i18ndc("lingmo_applet_org.kde.lingmo.notifications", "@action:button", "Send")
        enabled: replyTextField.length > 0
        onClicked: replyRow.replied(replyTextField.text)
    }
}
