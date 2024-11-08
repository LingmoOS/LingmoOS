/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.plasmoid

Item {
    id: root
    width: 640
    height: 480

    property Item toolBox

    function addApplet(applet, x, y) {
        var component = Qt.createComponent("PlasmoidContainer.qml")
        var plasmoidContainer = component.createObject(root, {"x": x, "y": y});

        plasmoidContainer.parent = root;
        plasmoidContainer.applet = applet
        applet.parent = plasmoidContainer
        applet.anchors.fill = plasmoidContainer
        applet.visible = true
        plasmoidContainer.visible = true

    }

    Containment.onAppletAdded: {
        addApplet(applet, x, y);
    }

    Component.onCompleted: {
        print("Test Containment loaded")
        print(plasmoid)
    }
}
