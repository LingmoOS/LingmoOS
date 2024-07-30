/*
    SPDX-FileCopyrightText: 2019 Carson Black <uhhadd@gmail.com>
    SPDX-FileCopyrightText: 2019 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Dialogs
import QtQuick.Controls 2.5 as QQC2
import org.kde.lingmoui 2.8 as LingmoUI

Item {
    property list<LingmoUI.Action> actions:[
        LingmoUI.Action {
            text: i18n("Open with External Program")
            icon.name: "document-open"
            onTriggered: Qt.openUrlExternally(Qt.url(`file://${preview.fullPath}`))
        },
        LingmoUI.Action {
            text: i18n("Open Containing Folder")
            icon.name: "document-open-folder"
            onTriggered: iconModel.openContainingFolder(preview.fullPath)
        },
        LingmoUI.Action {
            text: pickerMode ? i18n("Insert Name") : i18n("Copy Name to Clipboard")
            icon.name: "edit-copy"
            onTriggered: {
                clipboard(preview.iconName)
                cuttlefish.showPassiveNotification(i18n("Icon name copied to clipboard"), "short")
            }
        },
        LingmoUI.Action {
            id: screenshotAction
            icon.name: "camera-web-symbolic"
            text: i18n("Take Screenshot…")
            onTriggered: screenshotPopup.popup()
            LingmoUI.Action {
                text: i18n("Ocean Colors")
                onTriggered: cuttlefish.doScreenshot(Screenshot.MontageType.Normal)
            }
            LingmoUI.Action {
                text: i18n("Ocean Dark Colors")
                onTriggered: cuttlefish.doScreenshot(Screenshot.MontageType.Dark)
            }
            LingmoUI.Action {
                text: i18n("Ocean (Normal) and Ocean Dark")
                onTriggered: cuttlefish.doScreenshot(Screenshot.MontageType.Dual)
            }
            LingmoUI.Action {
                text: i18n("Active Color Scheme")
                onTriggered: cuttlefish.doScreenshot(Screenshot.MontageType.Active)
            }
        },
        LingmoUI.Action {
            text: i18n("View in Other Icon Themes")
            icon.name: "document-equal"
            onTriggered: comparison.open()
        }
    ]

    QQC2.Menu {
        id: screenshotPopup
        Repeater {
            model: screenshotAction.visibleChildren
            delegate: QQC2.MenuItem {
                action: modelData
            }
        }
    }

    function clipboard(text) {
        if (!pickerMode) {
            clipboardHelper.text = text;
            clipboardHelper.selectAll();
            clipboardHelper.copy();
        } else {
            iconModel.output(text);
        }
    }
    TextEdit {
        id: clipboardHelper
        visible: false
    }

}
