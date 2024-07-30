/*
 * SPDX-FileCopyrightText: 2012 Reza Fatahilah Shah <rshah0385@kireihana.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.1
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid

PlasmaCore.Dialog {
    id: root

    property alias image: comicPicture.image

    flags: Qt.Popup
    visible: false
    hideOnWindowDeactivate: true

    function toggleVisibility()
    {
        root.visible = !root.visible;
        if (root.visible) {
            Plasmoid.positionFullView(root);
            root.activateWindow;
        }
    }

    function close() {
        root.visible = false;
    }

    mainItem: PlasmaComponents.ScrollView {
        id: mainScrollArea

        anchors.fill: parent

        Layout.minimumWidth: comicPicture.nativeWidth
        Layout.maximumWidth: Layout.minimumWidth
        Layout.minimumHeight: comicPicture.nativeHeight
        Layout.maximumHeight: Layout.minimumHeight

        Flickable {
            id: viewContainer

            anchors.fill: parent

            contentWidth: comicPicture.nativeWidth
            contentHeight: comicPicture.nativeHeight

            //clip: true

            QImageItem {
                id: comicPicture

                anchors.fill: parent

                smooth: true
                fillMode: QImageItem.PreserveAspectFit

                MouseArea {
                    id: dialogMouseArea

                    anchors.fill: comicPicture

                    onClicked: {
                        root.close();
                    }
                }
            }
        }
    }
}
