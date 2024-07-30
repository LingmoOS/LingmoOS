/*
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.lingmoui as LingmoUI

LingmoUI.Dialog {
    id: dialog

    clip: true
    modal: true

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    header: LingmoUI.AbstractApplicationHeader {
        pageRow: null
        page: null

        minimumHeight: LingmoUI.Units.gridUnit * 1.6
        maximumHeight: LingmoUI.Units.gridUnit * 1.6
        preferredHeight: LingmoUI.Units.gridUnit * 1.6

        Keys.onEscapePressed: event => {
            if (dialog.opened) {
                dialog.close();
            } else {
                event.accepted = false;
            }
        }

        contentItem: RowLayout {
            width: parent.width
            LingmoUI.Heading {
                Layout.leftMargin: LingmoUI.Units.largeSpacing
                text: dialog.title
                elide: Text.ElideRight
            }
            Item {
                Layout.fillWidth: true
            }
            LingmoUI.Icon {
                id: closeIcon
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: LingmoUI.Units.largeSpacing
                Layout.preferredHeight: LingmoUI.Units.iconSizes.smallMedium
                Layout.preferredWidth: LingmoUI.Units.iconSizes.smallMedium
                source: closeMouseArea.containsMouse ? "window-close" : "window-close-symbolic"
                active: closeMouseArea.containsMouse
                MouseArea {
                    id: closeMouseArea
                    hoverEnabled: true
                    anchors.fill: parent
                    onClicked: mouse => dialog.close();
                }
            }
        }
    }

    contentItem: QQC2.Control {
        topPadding: 0
        leftPadding: 0
        rightPadding: 0
        bottomPadding: 0
    }
}
