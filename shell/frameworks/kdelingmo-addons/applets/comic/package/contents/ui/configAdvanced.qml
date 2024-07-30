/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.plasmoid
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    signal configurationChanged

    function saveConfig() {
        Plasmoid.showErrorPicture = showErrorPicture.checked;
        Plasmoid.maxComicLimit = maxComicLimit.value;

        Plasmoid.saveConfig();
        Plasmoid.configChanged();
    }

    LingmoUI.FormLayout {
        Component.onCompleted: {
            showErrorPicture.checked = Plasmoid.showErrorPicture;
            maxComicLimit.value = Plasmoid.maxComicLimit;
        }

        Layouts.RowLayout {
            LingmoUI.FormData.label: i18nc("@label:spinbox", "Comic cache:")

            Controls.SpinBox {
                id: maxComicLimit
                stepSize: 1
                onValueChanged: root.configurationChanged();
            }

            Controls.Label {
                text: i18ncp("@item:valuesuffix spacing to number + unit", "strip per comic", "strips per comic")
                textFormat: Text.PlainText
            }
        }

        Controls.CheckBox {
            id: showErrorPicture
            text: i18nc("@option:check", "Display error when downloading comic fails")
            onCheckedChanged: root.configurationChanged();
        }
    }
}
