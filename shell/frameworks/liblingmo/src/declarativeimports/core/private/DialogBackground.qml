/*
 *  SPDX-FileCopyrightText: 2023 Marco Martin <notmart@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.ksvg as KSvg

/**
 * Internal type used by DialogBackground in lingmoquick to load the actual
 * background SVG.
 */
KSvg.FrameSvgItem {
    anchors.fill: parent
    imagePath: "widgets/background"
}
