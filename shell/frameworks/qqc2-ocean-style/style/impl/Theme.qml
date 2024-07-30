/* SPDX-FileCopyrightText: 2023 Devin Lin <devin@kd.org>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import org.kde.lingmoui as LingmoUI

pragma Singleton

QtObject {

    // these are functions, not properties because we use LingmoUI.Theme, which is context dependent
    function separatorColor() {
        switch (LingmoUI.Theme.colorSet) {
            case LingmoUI.Theme.Button:
                return separatorColorHelper(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, 0.3);
            case LingmoUI.Theme.Selection:
                return LingmoUI.Theme.focusColor;
            default:
                return separatorColorHelper(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, 0.2);
        }
    }

    function buttonSeparatorColor() {
        return separatorColorHelper(LingmoUI.Theme.backgroundColor, LingmoUI.Theme.textColor, 0.3);
    }

    function separatorColorHelper(bg, fg, baseRatio) {
        if (LingmoUI.ColorUtils.brightnessForColor(bg) === LingmoUI.ColorUtils.Light) {
            return LingmoUI.ColorUtils.linearInterpolation(bg, fg, baseRatio);
        } else {
            return LingmoUI.ColorUtils.linearInterpolation(bg, fg, baseRatio / 2);
        }
    }
}
