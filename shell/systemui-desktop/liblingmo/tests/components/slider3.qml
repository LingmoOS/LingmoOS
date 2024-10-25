/*
 * SPDX-FileCopyrightText: 2019 Aleix Pol <aleixpol@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Layouts
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmoui as LingmoUI
// Run with qmlscene to use qqc2-desktop-style

ComponentBase {
    id: root
    title: "Lingmo Components 3 Slider"
    contentItem: GridLayout {
        columnSpacing: LingmoUI.Units.gridUnit
        rowSpacing: LingmoUI.Units.gridUnit
        columns: 2

        LingmoComponents.Label {
            text: "Horizontal slider"
        }
        LingmoComponents.Slider {
            id: horizontalSlider
            from: minSpinBox.value
            to: maxSpinBox.value
            stepSize: stepSizeSpinBox.value
        }

        LingmoComponents.Label {
            text: "Vertical slider"
        }
        LingmoComponents.Slider {
            id: verticalSlider
            from: minSpinBox.value
            to: maxSpinBox.value
            stepSize: stepSizeSpinBox.value
            orientation: Qt.Vertical
        }

        LingmoComponents.Label {
            text: "from: "
        }
        LingmoComponents.SpinBox {
            id: minSpinBox
            value: 0
            from: -999
            to: 999
            editable: true
        }

        LingmoComponents.Label {
            text: "to: "
        }
        LingmoComponents.SpinBox {
            id: maxSpinBox
            value: 100
            from: -999
            to: 999
            editable: true
        }

        LingmoComponents.Label {
            text: "stepSize: "
        }
        LingmoComponents.SpinBox {
            id: stepSizeSpinBox
            value: 1
            to: 999
            editable: true
        }
    }
}
