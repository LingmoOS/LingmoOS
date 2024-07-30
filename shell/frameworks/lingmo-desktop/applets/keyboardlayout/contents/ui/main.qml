/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt.labs.platform 1.1

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.workspace.components 2.0
import org.kde.lingmo.private.kcm_keyboard as KCMKeyboard
import org.kde.lingmoui 2.20 as LingmoUI

PlasmoidItem {
    id: root

    signal layoutSelected(int layoutIndex)

    preferredRepresentation: fullRepresentation
    toolTipMainText: Plasmoid.title
    toolTipSubText: fullRepresentationItem ? fullRepresentationItem.layoutNames.longName : ""

    fullRepresentation: KeyboardLayoutSwitcher {
        id: switcher

        hoverEnabled: true
        Plasmoid.status: hasMultipleKeyboardLayouts ? LingmoCore.Types.ActiveStatus : LingmoCore.Types.HiddenStatus

        LingmoCore.ToolTipArea {
            anchors.fill: parent
            mainText: root.toolTipMainText
            subText: root.toolTipSubText
        }

        Instantiator {
            id: actionsInstantiator
            model: switcher.keyboardLayout.layoutsList
            delegate: LingmoCore.Action {
                text: modelData.longName
                icon.icon: KCMKeyboard.Flags.getIcon(modelData.shortName)
                onTriggered: {
                    layoutSelected(index);
                }
            }
            onObjectAdded: (index, object) => {
                Plasmoid.contextualActions.push(object)
            }
            onObjectRemoved: (index, object) => {
                Plasmoid.contextualActions.splice(Plasmoid.contextualActions.indexOf(object), 1)
            }
        }

        Connections {
            target: root

            function onLayoutSelected(layoutIndex) {
               switcher.keyboardLayout.layout = layoutIndex;
            }
        }

        LingmoUI.Icon {
            id: flag

            anchors.fill: parent

            visible: valid && (Plasmoid.configuration.displayStyle === 1 || Plasmoid.configuration.displayStyle === 2)

            active: containsMouse
            source: KCMKeyboard.Flags.getIcon(layoutNames.shortName)

            BadgeOverlay {
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                visible: !countryCode.visible && Plasmoid.configuration.displayStyle === 2

                text: countryCode.text
                icon: flag
            }
        }

        LingmoComponents3.Label {
            id: countryCode
            anchors.fill: parent
            visible: Plasmoid.configuration.displayStyle === 0 || !flag.valid

            font.pointSize: height
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: layoutNames.displayName || layoutNames.shortName
            textFormat: Text.PlainText
        }
    }

    Plasmoid.onActivated: fullRepresentationItem.keyboardLayout.switchToNextLayout()

    function actionTriggered(actionName) {
        const layoutIndex = parseInt(actionName);
        if (!isNaN(layoutIndex)) {
            layoutSelected(layoutIndex);
        }
    }
}
