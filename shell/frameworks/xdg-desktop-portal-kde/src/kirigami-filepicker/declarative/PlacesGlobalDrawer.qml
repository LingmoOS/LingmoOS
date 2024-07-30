// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamifilepicker

/**
 * The PlacesGlobalDrawer type provides a GlobalDrawer containing common places on the file system
 */
Kirigami.OverlayDrawer {
    id: root

    signal placeOpenRequested(url place)

    handleClosedIcon.source: null
    handleOpenIcon.source: null
    width: Math.min(applicationWindow().width * 0.8, Kirigami.Units.gridUnit * 20)

    leftPadding: 0
    rightPadding: 0

    contentItem: ListView {
        spacing: 0
        model: FilePlacesModel {
            id: filePlacesModel
        }

        section.property: "group"
        section.delegate: Kirigami.Heading {
            leftPadding: Kirigami.Units.smallSpacing
            level: 6
            text: section
        }

        delegate: QQC2.ItemDelegate {
            required property string displayRole
            required property string iconName
            required property bool hidden
            required property url url

            visible: !hidden
            width: ListView.view.width
            text: displayRole
            icon.name: iconName
            onClicked: {
                root.placeOpenRequested(url)
            }
        }
    }
}
