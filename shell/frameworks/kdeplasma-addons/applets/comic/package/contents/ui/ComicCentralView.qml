/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.plasmoid

Item {
    id: root

    width: Kirigami.Units.gridUnit
    height: Kirigami.Units.gridUnit

    property variant comicData

    PlasmaComponents3.ToolButton {
        id: arrowLeft

        anchors {
            left: root.left
            verticalCenter: root.verticalCenter
        }

        icon.name: "go-previous"
        visible: (!Plasmoid.arrowsOnHover && (comicData.prev !== undefined))

        onClicked: {
            Plasmoid.updateComic(comicData.prev);
        }
    }
    PlasmaCore.ToolTipArea {
        id: tooltip
        anchors {
            left: arrowLeft.visible ? arrowLeft.right : root.left
            right: arrowRight.visible ? arrowRight.left : root.right
            leftMargin: arrowLeft.visible ? 4 : 0
            rightMargin: arrowRight.visible ? 4 : 0
            top: root.top
            bottom: root.bottom
        }
        subText: Plasmoid.comicData.additionalText
        active: subText

        MouseArea {
            id: comicImageArea

            anchors.fill: parent

            hoverEnabled: true
            preventStealing: false
            acceptedButtons: Qt.LeftButton | Qt.MiddleButton

            onClicked: {
                if (mouse.button == Qt.MiddleButton && Plasmoid.middleClick) {
                    fullDialog.toggleVisibility();
                }
            }

            ImageWidget {
                id: comicImage

                anchors.fill: parent
                enabled: false

                image: Plasmoid.comicData.image
                actualSize: Plasmoid.showActualSize
                isLeftToRight: Plasmoid.comicData.isLeftToRight
                isTopToBottom: Plasmoid.comicData.isTopToBottom
            }

            ButtonBar {
                id: buttonBar

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 10
                }

                visible: Plasmoid.arrowsOnHover && comicImageArea.containsMouse
                opacity: 0

                onPrevClicked: {
                    Plasmoid.updateComic(comicData.prev);
                }

                onNextClicked: {
                    Plasmoid.updateComic(comicData.next);
                }

                onZoomClicked: {
                    fullDialog.toggleVisibility();
                }

                states: State {
                    name: "show"; when: (Plasmoid.arrowsOnHover && comicImageArea.containsMouse)
                    PropertyChanges { target: buttonBar; opacity: 1; }
                }

                transitions: Transition {
                    from: ""; to: "show"; reversible: true
                    NumberAnimation { properties: "opacity"; duration: Kirigami.Units.longDuration; easing.type: Easing.InOutQuad }
                }
            }
        }
    }

    PlasmaComponents3.ToolButton {
        id: arrowRight

        anchors {
            right: root.right
            verticalCenter: root.verticalCenter
        }

        icon.name: "go-next"
        visible: (!Plasmoid.arrowsOnHover && (comicData.next !== undefined))

        onClicked: {
            Plasmoid.updateComic(comicData.next);
        }
    }

    FullViewWidget {
        id: fullDialog

        image: Plasmoid.comicData.image
    }
}
