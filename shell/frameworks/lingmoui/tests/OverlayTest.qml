/*
 *  SPDX-FileCopyrightText: 2022 Aleix Pol <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.lingmoui as LingmoUI

LingmoUI.ApplicationWindow {
    height: 720
    width: 360
    visible: true

    LingmoUI.OverlaySheet {
        id: sheet

        title: "Certificate Viewer"

        ColumnLayout {
            QQC2.DialogButtonBox {
                Layout.fillWidth: true

                QQC2.Button {
                    QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.ActionRole
                    text: "Export…"
                }

                QQC2.Button {
                    QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.DestructiveRole
                    text: "Close"
                    icon.name: "dialog-close"
                }
            }
        }
    }

    Timer {
        interval: 150
        running: true
        onTriggered: sheet.open()
    }
}
