/*
 *  SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.kirigami 2.20 as Kirigami

Rectangle {
    id: root

    color: "black"
    width: 640
    height: 480

    property Item containment

    SdkButtons {
        z: 1
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }

        containment: root.containment

        onFormFactor: formFactorType => {
            desktop.changeFormFactor(formFactorType);
        }
        onLocation: locationType => {
            desktop.changeLocation(locationType);
        }
        onRequestScreenshot: {
            desktop.takeScreenShot();
        }
    }

    onContainmentChanged: {
        print("New Containment:", containment)
        //containment.parent = root
        containment.visible = true
        containment.anchors.fill = root
    }

    Component.onCompleted: {
        desktop.stayBehind = true;
        desktop.fillScreen = true;
        print("View QML loaded");
    }
}
