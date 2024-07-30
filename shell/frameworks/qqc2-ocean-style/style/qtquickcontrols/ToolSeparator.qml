/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Controls.impl
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

import org.kde.ocean.impl as Impl

T.ToolSeparator {
    id: control
    // palette: LingmoUI.Theme.palette

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: false
    focusPolicy: Qt.NoFocus

    contentItem: LingmoUI.Separator {
        readonly property int defaultHeight: Impl.Units.mediumControlHeight
        implicitWidth: vertical ? 1 : defaultHeight
        implicitHeight: vertical ? defaultHeight : 1
    }
}
