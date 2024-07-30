/*
    SPDX-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

    Based on the clipboard applet:
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmo.core as LingmoCore
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.ksvg 1.0 as KSvg
import org.kde.kitemmodels 1.0 as KItemModels
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.extras 2.0 as LingmoExtras

import org.kde.lingmo.private.profiles 1.0

PlasmoidItem {
    id: main

    readonly property bool inPanel: (Plasmoid.location === LingmoCore.Types.TopEdge
        || Plasmoid.location === LingmoCore.Types.RightEdge
        || Plasmoid.location === LingmoCore.Types.BottomEdge
        || Plasmoid.location === LingmoCore.Types.LeftEdge)

    width: (Plasmoid.formFactor==LingmoCore.Types.Planar)? LingmoUI.Units.gridUnit * 14 : undefined
    height: (Plasmoid.formFactor==LingmoCore.Types.Planar)? LingmoUI.Units.gridUnit * 16: undefined

    switchWidth: LingmoUI.Units.gridUnit * 11
    switchHeight: LingmoUI.Units.gridUnit * 11
    Plasmoid.status: LingmoCore.Types.ActiveStatus
    Plasmoid.icon: inPanel ? "kate-symbolic" : "kate"
    toolTipMainText: i18n("Kate Sessions")

    Component.onCompleted: {
        plasmoid.removeAction("configure");
    }

    property var searchHeader: LingmoExtras.PlasmoidHeading {
        LingmoComponents3.TextField {
            id: filter
            placeholderText: i18n("Search…")
            clearButtonShown: true
            anchors.fill: parent
        }
    }

    fullRepresentation: LingmoComponents3.Page {

        id: dialogItem
        Layout.minimumWidth: LingmoUI.Units.gridUnit * 12
        Layout.minimumHeight: LingmoUI.Units.gridUnit * 12
        Layout.preferredWidth: LingmoUI.Units.gridUnit * 16
        Layout.preferredHeight: LingmoUI.Units.gridUnit * 24

        focus: true
        header: searchHeader

        property alias listMargins: listItemSvg.margins

        KSvg.FrameSvgItem {
            id : listItemSvg
            imagePath: "widgets/listitem"
            prefix: "normal"
            visible: false
        }

        Keys.onPressed: {
            switch(event.key) {
                case Qt.Key_Up: {
                    sessionsMenu.view.decrementCurrentIndex();
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Down: {
                    sessionsMenu.view.incrementCurrentIndex();
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Enter:
                case Qt.Key_Return: {
                    if (sessionsMenu.view.currentIndex >= 0) {
                        const profileIdentifier = sessionsMenu.model.get(sessionsMenu.view.currentIndex).profileIdentifier;
                        if (profileIdentifier) {
                            model.openProfile(profileIdentifier);
                            sessionsMenu.view.currentIndex = 0;
                        }
                    }
                    break;
                }
                case Qt.Key_Escape: {
                    if (filter.text == "") {
                        main.expanded = false;
                    } else {
                        filter.text = "";
                    }
                    event.accepted = true;
                    break;
                }
                default: { // forward key to filter
                    // filter.text += event.text wil break if the key is backspace
                    if (event.key == Qt.Key_Backspace && filter.text == "") {
                        return;
                    }
                    if (event.text != "" && !filter.activeFocus) {
                        sessionsMenu.view.currentIndex = -1
                        if (event.text == "v" && event.modifiers & Qt.ControlModifier) {
                            filter.paste();
                        } else {
                            filter.text = "";
                            filter.text += event.text;
                        }
                        filter.forceActiveFocus();
                        event.accepted = true;
                    }
                }
            }
        }
        ColumnLayout {
            anchors.fill: parent
            Menu {
                id: sessionsMenu
                model: KItemModels.KSortFilterProxyModel {
                    sourceModel: ProfilesModel {
                        id: model
                        appName: "kate"
                    }
                    filterRoleName: "name"
                    filterString: filter.text
                    filterCaseSensitivity: Qt.CaseInsensitive
                }
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: LingmoUI.Units.smallSpacing
                onItemSelected: function (profileIdentifier) {
                    model.openProfile(profileIdentifier)
                    main.expanded = false;
                }
            }
        }
    }
}
