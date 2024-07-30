/*
 *  SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Controls 2.5 as Controls
import QtQuick.Layouts 1.1 as Layouts

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    signal configurationChanged

    function saveConfig() {
        Plasmoid.arrowsOnHover = showArrowsOnOver.checked;
        Plasmoid.showComicTitle = showComicTitle.checked;
        Plasmoid.showComicIdentifier = showIdentifier.checked;
        Plasmoid.showComicAuthor = showAuthor.checked;
        Plasmoid.showComicUrl = showUrl.checked;

        Plasmoid.saveConfig();
        Plasmoid.configChanged();
    }

    Kirigami.FormLayout {
        Component.onCompleted: {
            showArrowsOnOver.checked = Plasmoid.arrowsOnHover;
            showComicTitle.checked = Plasmoid.showComicTitle;
            showIdentifier.checked = Plasmoid.showComicIdentifier;
            showAuthor.checked = Plasmoid.showComicAuthor;
            showUrl.checked = Plasmoid.showComicUrl;
        }

        Controls.CheckBox {
            id: showComicTitle
            Kirigami.FormData.label: i18nc ("Heading for showing various elements of a comic", "Show:")
            text: i18nc("@option:check", "Comic title")
            onCheckedChanged: root.configurationChanged();
        }

        Controls.CheckBox {
            id: showIdentifier
            text: i18nc("@option:check", "Comic identifier")
            onCheckedChanged: root.configurationChanged();
        }

        Controls.CheckBox {
            id: showAuthor
            text: i18nc("@option:check", "Comic author")
            onCheckedChanged: root.configurationChanged();
        }

        Controls.CheckBox {
            id: showUrl
            text: i18nc("@option:check", "Comic URL")
            onCheckedChanged: root.configurationChanged();
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        Controls.RadioButton {
            id: alwaysShowArrows
            Kirigami.FormData.label: i18n ("Show navigation buttons:")
            text: i18nc("@option:check", "Always")
            checked: !showArrowsOnOver.checked
            onCheckedChanged: root.configurationChanged();
        }

        Controls.RadioButton {
            id: showArrowsOnOver
            text: i18nc("@option:check", "Only on hover")
            onCheckedChanged: root.configurationChanged();
        }
    }
}
