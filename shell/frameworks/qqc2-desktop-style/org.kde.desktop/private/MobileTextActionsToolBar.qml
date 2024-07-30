/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

pragma Singleton
pragma ComponentBehavior: Bound

import QtQuick
import org.kde.kirigami as Kirigami

Loader {
    property /*TextInput | TextEdit*/ Item controlRoot
    property bool shouldBeVisible: false

    active: controlRoot !== null
        && shouldBeVisible
        && Kirigami.Settings.tabletMode
        && (controlRoot.selectedText.length > 0 || controlRoot.canPaste)

    Component.onCompleted: {
        // See https://bugreports.qt.io/browse/QTBUG-125071
        setSource(Qt.resolvedUrl("MobileTextActionsToolBarImpl.qml"), {
            controlRoot: Qt.binding(() => controlRoot),
        });
    }
}
