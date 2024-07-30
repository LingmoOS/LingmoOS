/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

ComponentBase {
    id: root
    title: "Plasma Components 3 TextField"
    property string longText: "This is a longer sentence"

    contentItem: Flow {
        spacing: Kirigami.Units.gridUnit

        PlasmaComponents.TextField {
            placeholderText: longText
        }

        PlasmaComponents.TextField {
            text: root.longText
        }

        PlasmaComponents.TextField {
            width: 400
            placeholderText: longText
        }

        PlasmaComponents.TextField {
            text: root.longText
            echoMode: TextInput.Password
        }
    }
}
