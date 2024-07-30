/*
    SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.ksvg 1.0 as KSvg


import org.kde.lingmo.private.containmentlayoutmanager 1.0 as ContainmentLayoutManager

ContainmentLayoutManager.ConfigOverlayWithHandles {
    id: overlay

    SequentialAnimation {
        id: removeAnim

        NumberAnimation {
            target: overlay.itemContainer
            property: "scale"
            from: 1
            to: 0
            duration: LingmoUI.Units.longDuration
            easing.type: Easing.InOutQuad
        }
        ScriptAction {
            script: {
                appletContainer.applet.plasmoid.internalAction("remove").trigger();
                appletContainer.editMode = false;
            }
        }
    }

    KSvg.FrameSvgItem {
        id: frame

        anchors.verticalCenter: parent.verticalCenter
        x: overlay.rightAvailableSpace > width + LingmoUI.Units.gridUnit
            ? parent.width + LingmoUI.Units.gridUnit
            : -width - LingmoUI.Units.gridUnit

        // This MouseArea is used to block input between the applet and the handle, to not make it steal by other applets
        MouseArea {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            z: -1
            x: overlay.rightAvailableSpace > parent.width + LingmoUI.Units.gridUnit ? -LingmoUI.Units.gridUnit : 0
            width: LingmoUI.Units.gridUnit + parent.width
            hoverEnabled: true
        }
        transform: Translate {
            x: open ? 0 : (overlay.rightAvailableSpace > frame.width + LingmoUI.Units.gridUnit ? -frame.width : frame.width)

            Behavior on x {
                NumberAnimation {
                    duration: LingmoUI.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        width: layout.implicitWidth + margins.left + margins.right
        height: Math.max(layout.implicitHeight + margins.top + margins.bottom, parent.height)
        imagePath: "widgets/background"

        ColumnLayout {
            id: layout
            anchors {
                fill: parent
                topMargin: parent.margins.top
                leftMargin: parent.margins.left
                bottomMargin: parent.margins.bottom
                rightMargin: parent.margins.right
            }

            ActionButton {
                id: rotateButton
                icon.name: "object-rotate-left-symbolic"
                toolTip: !rotateHandle.pressed ? i18n("Click and drag to rotate") : ""
                action: applet ? applet.plasmoid.internalAction("rotate") : null
                down: rotateHandle.pressed
                Component.onCompleted: {
                    if (action !== null) {
                        action.enabled = true;
                    }
                }
                MouseArea {
                    id: rotateHandle
                    anchors.fill: parent

                    property int startRotation
                    property real startCenterRelativeAngle

                    function pointAngle(pos: point): real {
                        var r = Math.sqrt(pos.x * pos.x + pos.y * pos.y);
                        var cosine = pos.x / r;

                        if (pos.y >= 0) {
                            return Math.acos(cosine) * (180/Math.PI);
                        } else {
                            return -Math.acos(cosine) * (180/Math.PI);
                        }
                    }

                    function centerRelativePos(x: real, y: real): point {
                        var mousePos = overlay.itemContainer.parent.mapFromItem(rotateButton, x, y);
                        var centerPos = overlay.itemContainer.parent.mapFromItem(overlay.itemContainer, overlay.itemContainer.width/2, overlay.itemContainer.height/2);

                        mousePos.x -= centerPos.x;
                        mousePos.y -= centerPos.y;
                        return mousePos;
                    }

                    onPressed: mouse => {
                        mouse.accepted = true;
                        startRotation = overlay.itemContainer.rotation;
                        startCenterRelativeAngle = pointAngle(centerRelativePos(mouse.x, mouse.y));
                    }

                    onPositionChanged: mouse => {
                        var rot = startRotation % 360;
                        var snap = 4;
                        var newRotation = Math.round(pointAngle(centerRelativePos(mouse.x, mouse.y)) - startCenterRelativeAngle + startRotation);

                        if (newRotation < 0) {
                            newRotation = newRotation + 360;
                        } else if (newRotation >= 360) {
                            newRotation = newRotation % 360;
                        }

                        snapIt(0);
                        snapIt(90);
                        snapIt(180);
                        snapIt(270);

                        function snapIt(snapTo) {
                            if (newRotation > (snapTo - snap) && newRotation < (snapTo + snap)) {
                                newRotation = snapTo;
                            }
                        }

                        overlay.itemContainer.rotation = newRotation;
                    }

                    onReleased: mouse => {
                        // save rotation
                        appletsLayout.save();
                    }
                }
            }

            ActionButton {
                icon.name: "configure"
                visible: qAction && qAction.enabled && (applet && applet.plasmoid.hasConfigurationInterface)
                qAction: applet ? applet.plasmoid.internalAction("configure") : null
                Component.onCompleted: {
                    if (qAction) {
                        qAction.enabled = true;
                    }
                }
            }

            ActionButton {
                // FIXME: missing from Ocean icons! See
                // https://bugs.kde.org/show_bug.cgi?id=472863.
                icon.name: "showbackground"
                toolTip: checked ? i18n("Hide Background") : i18n("Show Background")
                visible: (applet.plasmoid.backgroundHints & LingmoCore.Types.ConfigurableBackground)
                checked: applet.plasmoid.effectiveBackgroundHints & LingmoCore.Types.StandardBackground || applet.plasmoid.effectiveBackgroundHints & LingmoCore.Types.TranslucentBackground
                checkable: true
                onClicked: {
                    if (checked) {
                        if (applet.plasmoid.backgroundHints & LingmoCore.Types.StandardBackground || applet.plasmoid.backgroundHints & LingmoCore.Types.TranslucentBackground) {
                            applet.plasmoid.userBackgroundHints = applet.plasmoid.backgroundHints;
                        } else {
                            applet.plasmoid.userBackgroundHints = LingmoCore.Types.StandardBackground;
                        }
                    } else {
                        if (applet.plasmoid.backgroundHints & LingmoCore.Types.ShadowBackground || applet.plasmoid.backgroundHints & LingmoCore.Types.NoBackground) {
                            applet.plasmoid.userBackgroundHints = applet.plasmoid.backgroundHints;
                        } else {
                            applet.plasmoid.userBackgroundHints = LingmoCore.Types.ShadowBackground;
                        }
                    }
                }
            }

            MouseArea {
                drag.target: overlay.itemContainer
                Layout.minimumHeight: LingmoUI.Units.gridUnit * 3
                Layout.fillHeight: true
                Layout.fillWidth: true
                cursorShape: containsPress ? Qt.DragMoveCursor : Qt.OpenHandCursor
                hoverEnabled: true
                onPressed: mouse => {
                    appletsLayout.releaseSpace(overlay.itemContainer);
                }
                onPositionChanged: mouse => {
                    if (!pressed) {
                        return;
                    }
                    appletsLayout.showPlaceHolderForItem(overlay.itemContainer);
                    var dragPos = mapToItem(overlay.itemContainer, mouse.x, mouse.y);
                    overlay.itemContainer.userDrag(Qt.point(overlay.itemContainer.x, overlay.itemContainer.y), dragPos);
                }
                onReleased: mouse => {
                    appletsLayout.hidePlaceHolder();
                    appletsLayout.positionItem(overlay.itemContainer);
                }
            }

            ActionButton {
                id: closeButton
                icon.name: "edit-delete-remove"
                toolTip: i18n("Remove")
                visible: {
                    if (!applet) {
                        return false;
                    }
                    var a = applet.plasmoid.internalAction("remove");
                    return a && a.enabled || false;
                }
                // we don't set action, since we want to catch the button click,
                // animate, and then trigger the "remove" action
                // Triggering the action is handled in the overlay.itemContainer, we just
                // Q_EMIT a signal here to avoid the applet-gets-removed-before-we-
                // can-animate it race condition.
                onClicked: {
                    removeAnim.restart();
                }
                Component.onCompleted: {
                    var a = applet.plasmoid.internalAction("remove");
                    if (a) {
                        a.enabled = true;
                    }
                }
            }
        }
    }
}
