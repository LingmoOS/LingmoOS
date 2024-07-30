/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.lingmo.core as LingmoCore
import org.kde.lingmo.components as LingmoComponents
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.lingmoui as LingmoUI

// IconTab

LingmoComponents.Page {
    id: iconsPage
    anchors {
        fill: parent
        margins: _s
    }
    Column {
        anchors.fill: parent
        spacing: _s

        LingmoUI.Heading {
            width: parent.width
            elide: Text.ElideRight
            level: 1
            text: "Icons"
        }
        LingmoComponents.Label {
            text: "iconSizes.small  : " + LingmoUI.Units.iconSizes.small +
                    ", iconSizes.desktop: " + LingmoUI.Units.iconSizes.desktop +
                    ",<br />iconSizes.toolbar: " + LingmoUI.Units.iconSizes.toolbar +
                    ", iconSizes.dialog : " + LingmoUI.Units.iconSizes.dialog

        }
        Flow {
            //height: _h
            width: parent.width
            spacing: _s

            LingmoUI.Icon {
                source: "configure"
                width: _h
                height: width
            }
            LingmoUI.Icon {
                source: "dialog-ok"
                width: _h
                height: width
            }
            LingmoUI.Icon {
                source: "folder-green"
                width: _h
                height: width
            }
            LingmoUI.Icon {
                source: "akonadi"
                width: _h
                height: width
            }
            LingmoUI.Icon {
                source: "clock"
                width: _h
                height: width
            }
            KQuickControlsAddons.QIconItem {
                icon: "preferences-desktop-icons"
                width: _h
                height: width
            }

        }
        LingmoUI.Heading {
            level: 2
            text: "ToolTip"
        }

        Row {
            spacing: _s
            LingmoCore.ToolTipArea {
                width: childrenRect.width
                height: childrenRect.height
                icon: "klipper"
                mainText: "Fish sighted in the wild, in the wild, a fish was seen."
                subText: "A mean-looking grouper swam by."
                LingmoUI.Icon {
                    id: akonadiIcon
                    objectName: "akonadiIcon"
                    source: "akonadi"
                    width: height
                    height: _h
                    //anchors.horizontalCenter: parent.horizontalCenter
                    Rectangle { color: "orange"; opacity: 0.3; anchors.fill: parent; }
                }
            }
            LingmoCore.ToolTipArea {
                height: _h
                width: height
                image: bridgeimage.source
                mainText: "Bridge"
                subText: "Waalbrug."
                Image {
                    id: bridgeimage
                    objectName: "bridgeimage"
                    height: _h
                    width: height
                    fillMode: Image.PreserveAspectFit
                    source: "../images/bridge.jpg"
                }
            }
            LingmoCore.ToolTipArea {
                width: childrenRect.width
                height: childrenRect.height
                mainItem: LingmoComponents.Label {
                    text: "Nijmegen North Beach"
                    anchors.centerIn: parent
                }
                Image {
                    objectName: "surfboardimage"
                    height: _h
                    width: height
                    fillMode: Image.PreserveAspectFit
                    source: "../images/surfboard.jpg"

                    //subText: "A surfboard on the beach. <br />The photo shows the Waal river's north beach, \
                    //across the water from Nijmegen, Netherlands. It was taken during the summer festivals a few years back."
                }
            }
            LingmoCore.ToolTipArea {
                width: childrenRect.width
                height: childrenRect.height
                mainText: "Tooltip on button"
                LingmoComponents.Button {
                    id: button
                    text: "Button"
                    iconSource: "call-start"
                }
            }


        }
    }
}
