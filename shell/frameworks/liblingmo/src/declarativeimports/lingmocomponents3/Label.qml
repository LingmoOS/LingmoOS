/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

T.Label {
    id: control

    // Work around Qt bug where left aligned text is not right aligned
    // in RTL mode unless horizontalAlignment is explicitly set.
    // https://bugreports.qt.io/browse/QTBUG-95873
    horizontalAlignment: Text.AlignLeft

    activeFocusOnTab: false

    //font data is the system one by default
    color: LingmoUI.Theme.textColor
    linkColor: LingmoUI.Theme.linkColor

    opacity: enabled ? 1 : 0.6

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
