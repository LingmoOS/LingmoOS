/*
 *  SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0
import org.kde.draganddrop 2.0 as DnD

import "items"

ContainmentItem {
    id: root

    //be at least the same size as the system tray popup
    Layout.minimumWidth: Kirigami.Units.gridUnit * 24
    Layout.minimumHeight: Kirigami.Units.gridUnit * 21
    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight * 1.5

    property Component plasmoidItemComponent

    Containment.onAppletAdded: {
        addApplet(applet);
        //when we add an applet, select it straight away
        //we know it will always be at the end of the stack
        tabbar.currentIndex = mainStack.count -1;
    }
    Containment.onAppletRemoved: {
        for (var i = 0; i < mainStack.count; i++) {
            if (mainStack.children[i].itemId === applet.id) {
                mainStack.children[i].destroy();
                break;
            }
        }
    }

    function addApplet(applet) {
        const appletItem = root.itemFor(applet);

        if (!plasmoidItemComponent) {
            plasmoidItemComponent = Qt.createComponent("items/PlasmoidItem.qml");
        }

        if (plasmoidItemComponent.status === Component.Error) {
            console.warn("Could not create PlasmoidItem", plasmoidItemComponent.errorString());
        }

        var plasmoidContainer = plasmoidItemComponent.createObject(mainStack, { "applet": appletItem });

        appletItem.anchors.fill = undefined;
        appletItem.parent = plasmoidContainer;
        appletItem.anchors.fill = plasmoidContainer;
        appletItem.visible = true;
    }

    Component.onCompleted: {
        var applets = Containment.applets;
        for (var i = 0 ; i < applets.length; i++) {
            addApplet(applets[i]);
        }
    }

    Item {
        anchors.fill: parent
        PlasmaComponents.TabBar {
            id: tabbar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            Repeater {
                model: mainStack.children

                //attached properties:
                //  model == a QQmlDMObjectData wrapper round the PlasmoidItem
                //  modelData == the PlasmoidItem instance
                PlasmaComponents.TabButton {
                    text: model.text
                    MouseArea {
                        acceptedButtons: Qt.RightButton
                        anchors.fill: parent
                        onClicked: {
                            modelData.clicked(mouse);
                        }
                    }
                }
            }
            //hack: PlasmaComponents.TabBar is being weird with heights. Probably a bug
            height: contentChildren[0] ? contentChildren[0].height : undefined
        }

        StackLayout {
            id: mainStack
            currentIndex: tabbar.currentIndex
            anchors.top: tabbar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }
    }

    DnD.DropArea {
        anchors.fill: parent

        preventStealing: true

        /** Extracts the name of the applet in the drag data if present
         * otherwise returns null*/
        function appletName(event) {
            if (event.mimeData.formats.indexOf("text/x-plasmoidservicename") < 0) {
                return null;
            }
            var plasmoidId = event.mimeData.getDataAsByteArray("text/x-plasmoidservicename");
            return plasmoidId;
        }

        onDragEnter: {
            if (!appletName(event)) {
                event.ignore();
            }
        }

        onDrop: {
            var plasmoidId = appletName(event);
            if (!plasmoidId) {
                event.ignore();
                return;
            }
            plasmoid.newTask(plasmoidId);
        }
    }

    PlasmaComponents.Label {
        anchors.fill: mainStack
        text: i18n("Drag applets here")
        textFormat: Text.PlainText
        visible: mainStack.count === 0
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
