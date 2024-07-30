/*
    SPDX-FileCopyrightText: 2024 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

// Sensor face subtype intended to be use for compact representations of faces
SensorFace {
    // Convenience properties to make sizing logic for faces simpler
    readonly property bool horizontalFormFactor: formFactor == SensorFace.Horizontal
    readonly property bool verticalFormFactor: formFactor == SensorFace.Vertical
    readonly property bool constrainedFormFactor: formFactor == SensorFace.Constrained
    readonly property real goldenRatio: 1.618
    readonly property real defaultMinimumSize: Kirigami.Units.gridUnit

    Layout.minimumWidth: defaultMinimumSize
    Layout.minimumHeight: defaultMinimumSize

    // By default, prefer a rectangular size base on the golden ratio for horizontal panels
    Layout.preferredWidth: horizontalFormFactor ? Math.min(Math.max(height * goldenRatio, Layout.minimumWidth), Layout.maximumWidth) : -1
    // For vertical panels, just try to keep things square since increasing height usually does not make sense
    Layout.preferredHeight: verticalFormFactor ? Math.min(Math.max(width, Layout.minimumHeight), Layout.maximumHeight) : -1

    // Limit the maximum size to a reasonably sensible value. This matches what some Plasmoids do.
    Layout.maximumWidth: horizontalFormFactor ? Math.max(Kirigami.Units.iconSizes.enormous, Layout.minimumWidth) : -1
    Layout.maximumHeight: verticalFormFactor ? Math.max(Kirigami.Units.iconSizes.enormous, Layout.minimumHeight) : -1
}
