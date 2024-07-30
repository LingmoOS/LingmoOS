/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.kquickcontrolsaddons 2.0
import org.kde.lingmo.plasmoid

Item {
    id: root

    implicitWidth: 10
    implicitHeight: comicIdentifier.height

    property bool showUrl: false
    property bool showIdentifier: false
    property variant comicData

    visible: (comicIdentifier.text.length > 0 || comicUrl.text.length > 0)

    LingmoComponents3.Label {
        id: comicIdentifier

        anchors {
            left: root.left
            top: root.top
            bottom: root.bottom
            right: comicUrl.left
            leftMargin: 2
        }

        color: LingmoUI.Theme.textColor
        visible: (showIdentifier && comicIdentifier.text.length > 0)
        text: (showIdentifier && comicData.currentReadable != undefined) ? comicData.currentReadable : ""
        textFormat: Text.PlainText

        MouseArea {
            id: idLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = LingmoUI.Theme.highlightColor;
            }

            onExited: {
                parent.color = LingmoUI.Theme.textColor;
            }

            onClicked: {
                Plasmoid.goJump();
            }

            LingmoCore.ToolTipArea {
                anchors.fill: idLabelArea
                mainText: i18nc("@info:tooltip", "Jump to strip…")
            }
        }
    }

    LingmoComponents3.Label {
        id:comicUrl

        anchors {
            top: root.top
            bottom: root.bottom
            right: root.right
            rightMargin: 2
        }

        color: LingmoUI.Theme.textColor
        visible: (showUrl && comicUrl.text.length > 0)
        text: (showUrl && comicData.websiteHost.length > 0) ? comicData.websiteHost : ""
        textFormat: Text.PlainText

        MouseArea {
            id: idUrlLabelArea

            anchors.fill: parent

            hoverEnabled: true

            onEntered: {
                parent.color = LingmoUI.Theme.highlightColor;
            }

            onExited: {
                parent.color = LingmoUI.Theme.textColor;
            }

            onClicked: {
                Plasmoid.shop();
            }

            LingmoCore.ToolTipArea {
                anchors.fill: idUrlLabelArea
                mainText: i18nc("@info:tooltip", "Visit the comic website")
            }
        }
    }
}
