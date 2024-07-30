/*
 *   SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import org.kde.lingmo.core as LingmoCore
import Qt5Compat.GraphicalEffects

import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.private.nanoshell 2.0 as NanoShell

pragma Singleton

Window {
    id: window

    flags: Qt.FramelessWindowHint
    property alias backgroundColor: background.color
    LingmoUI.ImageColors {
        id: colorGenerator
        source: icon.source
    }

    function open(splashIcon, title, x, y, sourceIconSize, color) {
        iconParent.scale = sourceIconSize/iconParent.width;
        background.scale = 0;
        backgroundParent.x = -window.width/2 + x
        backgroundParent.y = -window.height/2 + y
        window.title = title;
        icon.source = splashIcon;

        if (color !== undefined) {
            // Break binding to use custom color
            background.color = color
        } else {
            // Recreate binding
            background.color = Qt.binding(function() { return colorGenerator.dominant})
        }

        background.state = "open";
    }

    Connections {
        target: NanoShell.StartupNotifier
        enabled: NanoShell.StartupNotifier.isValid

        function onActivationStarted(appId, iconName) {
            icon.source = iconName
            background.state = "open";
        }
    }

    property alias state: background.state
    property alias icon: icon.source

    width: Screen.width
    height: Screen.height
    color: "transparent"
    onVisibleChanged: {
        if (!visible) {
            background.state = "closed";
        }
    }
    onActiveChanged: {
        if (!active) {
            background.state = "closed";
        }
    }


    Item {
        id: backgroundParent
        width: window.width
        height: window.height

        Item {
            id: iconParent
            z: 2
            anchors.centerIn: background
            width: LingmoUI.Units.iconSizes.enormous
            height: width
            LingmoUI.Icon {
                id: icon
                anchors.fill:parent
                LingmoUI.Theme.colorSet: LingmoUI.Theme.ComplementaryColorGroup
            }
            DropShadow {
                anchors.fill: icon
                horizontalOffset: 0
                verticalOffset: 0
                radius: 8.0
                samples: 17
                color: "#80000000"
                source: icon
            }
        }

        Rectangle {
            id: background
            anchors.fill: parent

            color: colorGenerator.dominant

            state: "closed"

            states: [
                State {
                    name: "closed"
                    PropertyChanges {
                        target: window
                        visible: false
                    }
                },
                State {
                    name: "open"

                    PropertyChanges {
                        target: window
                        visible: true
                    }
                }
            ]

            transitions: [
                Transition {
                    from: "closed"
                    SequentialAnimation {
                        ScriptAction {
                            script: { 
                                window.showMaximized();
                            }
                        }
                        ParallelAnimation {
                            ScaleAnimator {
                                target: background
                                from: background.scale
                                to: 1
                                duration: LingmoUI.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            ScaleAnimator {
                                target: iconParent
                                from: iconParent.scale
                                to: 1
                                duration: LingmoUI.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            XAnimator {
                                target: backgroundParent
                                from: backgroundParent.x
                                to: 0
                                duration: LingmoUI.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            YAnimator {
                                target: backgroundParent
                                from: backgroundParent.y
                                to: 0
                                duration: LingmoUI.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
            ]
        }
    }
}
