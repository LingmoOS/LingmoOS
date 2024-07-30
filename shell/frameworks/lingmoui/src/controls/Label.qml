/*
 * Copyright (C) 2024 Lingmo OS Team <team@lingmo.org>
 *
 * Author: Lingmo OS Team <team@lingmo.org>
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */

import QtQuick
import QtQuick.Window
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.Label {
    id: control

    verticalAlignment: lineCount > 1 ? Text.AlignTop : Text.AlignVCenter

    activeFocusOnTab: false
    // Text.NativeRendering is broken on non integer pixel ratios
    // renderType: Window.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    renderType: Text.QtRendering

    font.capitalization: LingmoUI.Theme.defaultFont.capitalization
    font.family: "Noto Sans"
    font.italic: LingmoUI.Theme.defaultFont.italic
    font.letterSpacing: LingmoUI.Theme.defaultFont.letterSpacing
    font.pointSize: 12
    font.strikeout: LingmoUI.Theme.defaultFont.strikeout
    font.underline: LingmoUI.Theme.defaultFont.underline
    font.weight: LingmoUI.Theme.defaultFont.weight
    font.wordSpacing: LingmoUI.Theme.defaultFont.wordSpacing
    color: LingmoUI.Theme.textColor
    linkColor: LingmoUI.Theme.linkColor

    opacity: enabled ? 1 : 0.6

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
