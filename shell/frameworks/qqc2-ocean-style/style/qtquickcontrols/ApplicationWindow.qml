/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.ApplicationWindow {
    id: window
    // palette: LingmoUI.Theme.palette
    LingmoUI.Theme.colorSet: LingmoUI.Theme.Window
    color: LingmoUI.Theme.backgroundColor
}
