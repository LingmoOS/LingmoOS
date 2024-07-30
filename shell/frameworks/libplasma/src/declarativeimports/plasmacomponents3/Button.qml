/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import "private" as Private
//NOTE: importing PlasmaCore is necessary in order to make KSvg load the current Plasma Theme
import org.kde.plasma.core as PlasmaCore

T.Button {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    topPadding: (background as Private.ButtonBackground)?.topMargin ?? undefined
    leftPadding: (background as Private.ButtonBackground)?.leftMargin ?? undefined
    rightPadding: (background as Private.ButtonBackground)?.rightMargin ?? undefined
    bottomPadding: (background as Private.ButtonBackground)?.bottomMargin ?? undefined

    spacing: Kirigami.Units.smallSpacing

    hoverEnabled: !Kirigami.Settings.tabletMode

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.Kirigami.MnemonicData.sequence
        onActivated: control.clicked()
    }

    Kirigami.Theme.inherit: flat
    Kirigami.Theme.colorSet: Kirigami.Theme.Button

    contentItem: Private.ButtonContent {
        labelText: control.Kirigami.MnemonicData.richTextLabel
        button: control
    }

    background: Private.ButtonBackground {
        button: control
    }
}
