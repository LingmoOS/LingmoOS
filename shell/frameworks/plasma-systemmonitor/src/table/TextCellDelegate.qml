/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami

BaseCellDelegate {
    id: delegate

    leftPadding: Kirigami.Units.smallSpacing

    contentItem: Label {
        id: label
        text: delegate.text
        maximumLineCount: 1
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
