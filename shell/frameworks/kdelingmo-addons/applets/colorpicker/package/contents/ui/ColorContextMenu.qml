/*
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.extras as LingmoExtras

/**
 * Context menu to copy colors in different formats.
 */
LingmoExtras.ModelContextMenu {
    id: formattingMenu

    required property QtObject picker
    required property Item colorLabel
    required property Item copyIndicatorLabel
    required property Timer colorLabelRestoreTimer

    placement: LingmoExtras.Menu.BottomPosedLeftAlignedPopup

    onClicked: {
        picker.copyToClipboard(model.text)
        colorLabel.visible = false;
        copyIndicatorLabel.visible = true;
        colorLabelRestoreTimer.start()
    }

    onStatusChanged: {
        if (status === LingmoExtras.Menu.Closed) {
            formattingMenu.destroy();
        }
    }
}
