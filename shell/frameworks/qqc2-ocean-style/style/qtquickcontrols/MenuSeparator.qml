/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.MenuSeparator {
    id: control
    // palette: LingmoUI.Theme.palette

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: false
    focusPolicy: Qt.NoFocus

    topPadding: LingmoUI.Units.smallSpacing / 2
    bottomPadding: LingmoUI.Units.smallSpacing / 2

    contentItem: LingmoUI.Separator {
        implicitWidth: LingmoUI.Units.gridUnit * 8
        implicitHeight: 1
    }
}
