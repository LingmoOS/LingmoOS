/*
 *   SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid 2.0

PlasmoidItem {
    id: root

    Layout.minimumWidth: internalContainmentItem ? internalContainmentItem.Layout.minimumWidth : 0
    Layout.minimumHeight: internalContainmentItem ? internalContainmentItem.Layout.minimumHeight : 0
    Layout.preferredHeight: Layout.minimumHeight

    preferredRepresentation: fullRepresentation
    Plasmoid.status: internalContainmentItem ? internalContainmentItem.status : PlasmaCore.Types.UnknownStatus

    property Item internalContainmentItem

    Component.onCompleted: {
        root.internalContainmentItem = plasmoid.internalContainmentItem;

        if (root.internalContainmentItem === null) {
            return;
        }
        root.internalContainmentItem.anchors.fill = undefined;
        root.internalContainmentItem.parent = root;
        root.internalContainmentItem.anchors.fill = root;
    }

    Connections {
        target: plasmoid
        function onInternalContainmentItemChanged() {
            root.internalContainmentItem = plasmoid.internalContainmentItem;
            root.internalContainmentItem.parent = root;
            root.internalContainmentItem.anchors.fill = root;
        }
    }
}
