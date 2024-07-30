/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.plasmoid 2.0

//SystemTray is a Containment. To have it presented as a widget in Lingmo we need thin wrapping applet
PlasmoidItem {
    id: root

    property ContainmentItem internalSystray

    Layout.minimumWidth: internalSystray ? internalSystray.Layout.minimumWidth : 0
    Layout.minimumHeight: internalSystray ? internalSystray.Layout.minimumHeight : 0
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight

    preferredRepresentation: fullRepresentation
    Plasmoid.status: internalSystray ? internalSystray.plasmoid.status : LingmoCore.Types.UnknownStatus

    //synchronize state between SystemTray and wrapping Applet
    onExpandedChanged: {
        if (internalSystray) {
            internalSystray.expanded = root.expanded
        }
    }
    Connections {
        target: internalSystray
        function onExpandedChanged() {
            root.expanded = internalSystray.expanded
        }
    }

    Component.onCompleted: {
        root.internalSystray = Plasmoid.internalSystray;

        if (root.internalSystray == null) {
            return;
        }
        root.internalSystray.parent = root;
        root.internalSystray.anchors.fill = root;
    }

    Connections {
        target: Plasmoid
        function onInternalSystrayChanged() {
            root.internalSystray = Plasmoid.internalSystray;
            root.internalSystray.parent = root;
            root.internalSystray.anchors.fill = root;
        }
    }
}
