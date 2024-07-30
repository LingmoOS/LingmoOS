/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami

Kirigami.ImageColors {
    id: imageColors

    required property Item loadingIndicator

    onPaletteChanged: {
        if (imageColors.palette.length > 0) {
            root.addColorToHistory(imageColors.average);
        }
        loadingIndicator.jobDone();
        imageColors.destroy();
    }
}
