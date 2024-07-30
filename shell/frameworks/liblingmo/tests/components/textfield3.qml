/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI

ComponentBase {
    id: root
    title: "Lingmo Components 3 TextField"
    property string longText: "This is a longer sentence"

    contentItem: Flow {
        spacing: LingmoUI.Units.gridUnit

        LingmoComponents.TextField {
            placeholderText: longText
        }

        LingmoComponents.TextField {
            text: root.longText
        }

        LingmoComponents.TextField {
            width: 400
            placeholderText: longText
        }

        LingmoComponents.TextField {
            text: root.longText
            echoMode: TextInput.Password
        }
    }
}
