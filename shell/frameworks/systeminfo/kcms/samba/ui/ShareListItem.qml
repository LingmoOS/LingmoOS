// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

import QtQuick 2.14
import org.kde.lingmoui 2.12 as LingmoUI
import QtQuick.Controls 2.14 as QQC2
import QtQuick.Layouts 1.14

QQC2.ItemDelegate {
    id: listItem

    contentItem: RowLayout {
        spacing: LayoutMirroring.enabled ? listItem.rightPadding : listItem.leftPadding

        ColumnLayout {
            spacing: 0
            Layout.fillWidth: true

            RowLayout {
                QQC2.Label {
                    elide: Text.ElideMiddle
                    horizontalAlignment: Text.AlignLeft
                    font: LingmoUI.Theme.smallFont
                    text: i18nc("@label local file system path", 'Path:')
                }

                LingmoUI.UrlButton {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    font: LingmoUI.Theme.smallFont
                    url: ROLE_Path
                }
            }

            RowLayout {
                QQC2.Label {
                    elide: Text.ElideMiddle
                    horizontalAlignment: Text.AlignLeft
                    font: LingmoUI.Theme.smallFont
                    text: i18nc("@label labels a samba url or path", 'Shared at:')
                }

                // either fully qualified url
                LingmoUI.UrlButton {
                    id: link
                    visible: ROLE_ShareUrl !== undefined
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignLeft
                    font: LingmoUI.Theme.smallFont
                    url: ROLE_ShareUrl !== undefined ? ROLE_ShareUrl : ""
                }

                // ... or name when we couldn't resolve a fully qualified url
                QQC2.Label {
                    visible: !link.visible
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                    horizontalAlignment: Text.AlignLeft
                    font: LingmoUI.Theme.smallFont
                    text: "/" + ROLE_Name
                }
            }
        }

        QQC2.ToolButton {
            action: LingmoUI.Action {
                id: propertiesAction
                icon.name: "document-properties"
                tooltip: xi18nc("@info:tooltip", "Open folder properties to change share settings")
                displayHint: LingmoUI.DisplayHint.IconOnly
                onTriggered: source => view.model.showPropertiesDialog(model.row)
            }

            QQC2.ToolTip {
                text: propertiesAction.tooltip
            }
        }
    }
}
