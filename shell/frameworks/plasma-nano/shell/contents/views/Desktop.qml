/*
 *  SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami

Rectangle {
    id: root

    visible: false //adjust borders is run during setup. We want to avoid painting till completed
    property Item containment

    color: containment && containment.plasmoid.backgroundHints == PlasmaCore.Types.NoBackground ? "transparent" : Kirigami.Theme.textColor

    function toggleWidgetExplorer(containment) {
        console.log("Widget Explorer toggled");
        if (widgetExplorerStack.source != "") {
            widgetExplorerStack.source = "";
        } else {
            widgetExplorerStack.setSource(Qt.resolvedUrl("../explorer/WidgetExplorer.qml"), {"containment": containment, "containmentInterface": root.containment})
        }
    }

    Loader {
        id: widgetExplorerStack
        z: 99
        asynchronous: true
        anchors.fill: parent

        onLoaded: {
            if (widgetExplorerStack.item) {
                item.closed.connect(function() {
                    widgetExplorerStack.source = ""
                });

                item.topPanelHeight = containment.availableScreenRect.y
                item.bottomPanelHeight = root.height - (containment.availableScreenRect.height + containment.availableScreenRect.y)

                item.leftPanelWidth = containment.availableScreenRect.x
                item.rightPanelWidth = root.width - (containment.availableScreenRect.width + containment.availableScreenRect.x)
            }
        }
    }

    onContainmentChanged: {
        containment.parent = root;
        containment.visible = true;
        containment.anchors.fill = root;
    }

    Component.onCompleted: {
        visible = true
    }
}
