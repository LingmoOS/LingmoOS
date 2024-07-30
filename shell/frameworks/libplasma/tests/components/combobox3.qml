/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components
import org.kde.kirigami as Kirigami

ComponentBase {
    id: root
    title: "Plasma Components 3 ComboBox"
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.gridUnit
        ListModel {
            id: demoModel
            ListElement { text: "Banana"; color: "Yellow" }
            ListElement { text: "Apple"; color: "Green" }
            ListElement { text: "Coconut"; color: "Brown" }
        }

        ComboBox {
            model:demoModel
            textRole: "text"
        }
        ComboBox {
            editable: true
            model: demoModel
            textRole: "text"
        }
    }
}
