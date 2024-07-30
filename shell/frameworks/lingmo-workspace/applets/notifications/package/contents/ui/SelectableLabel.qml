/*
    SPDX-FileCopyrightText: 2011 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2014, 2019 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts 1.1

import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmoui 2.20 as LingmoUI

import org.kde.lingmo.private.notifications as Notifications

LingmoComponents3.ScrollView {
    id: bodyTextContainer

    property alias text: bodyText.text

    property int cursorShape

    property QtObject contextMenu: null
    property ListView listViewParent: null

    signal clicked(var mouse)
    signal linkActivated(string link)

    leftPadding: mirrored && !LingmoUI.Settings.isMobile ? LingmoComponents3.ScrollBar.vertical.width : 0
    rightPadding: !mirrored && !LingmoUI.Settings.isMobile ? LingmoComponents3.ScrollBar.vertical.width : 0

    LingmoComponents3.TextArea {
        id: bodyText
        enabled: !LingmoUI.Settings.isMobile
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        background: null
        color: LingmoUI.Theme.textColor

        // Selectable only when we are in desktop mode
        selectByMouse: !LingmoUI.Settings.tabletMode

        readOnly: true
        wrapMode: TextEdit.Wrap
        textFormat: TextEdit.RichText

        onLinkActivated: bodyTextContainer.linkActivated(link)

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: bodyTextContainer.clicked(null)
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            cursorShape: {
                if (bodyText.hoveredLink) {
                    return Qt.PointingHandCursor;
                } else if (bodyText.selectionStart !== bodyText.selectionEnd) {
                    return Qt.IBeamCursor;
                } else {
                    return bodyTextContainer.cursorShape || Qt.IBeamCursor;
                }
            }
            onTapped: eventPoint => {
                contextMenu = contextMenuComponent.createObject(bodyText);
                contextMenu.link = bodyText.linkAt(eventPoint.position.x, eventPoint.position.y);

                contextMenu.closed.connect(function() {
                    contextMenu.destroy();
                    contextMenu = null;
                });
                contextMenu.open(eventPoint.position.x, eventPoint.position.y);
            }
        }
    }

    Component {
        id: contextMenuComponent

        EditContextMenu {
            target: bodyText
        }
    }

    Component.onCompleted: if (bodyTextContainer.listViewParent !== null) {
        bodyTextContainer.listViewParent.wheelForwarder.interceptWheelEvent(bodyText);
    }
}
