/*
 *   SPDX-FileCopyrightText: 2011, 2012 Shaun Reich <shaun.reich@kdemail.net>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.kitemmodels 1.0 as KItemModels
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.profiles 1.0 as Profiles

PlasmoidItem {
   id: konsoleProfiles

    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)

    switchWidth: Kirigami.Units.gridUnit * 11
    switchHeight: Kirigami.Units.gridUnit * 9

    Layout.minimumWidth: Kirigami.Units.gridUnit * 12
    Layout.minimumHeight: Kirigami.Units.gridUnit * 10

    Plasmoid.icon: inPanel ? "dialog-scripts-symbolic" : "utilities-terminal"

    fullRepresentation: FocusScope {
        anchors.fill: parent

        Connections {
            target: konsoleProfiles
            function onExpandedChanged() {
                if (konsoleProfiles.expanded) {
                    view.forceActiveFocus();
                }
            }
        }

        KItemModels.KSortFilterProxyModel {
            id: sortModel
            sortRoleName: "name"
            sortOrder: Qt.AscendingOrder
            sourceModel: Profiles.ProfilesModel {
                id: profilesModel
                appName: "konsole"
            }
        }

        Row {
            id: headerRow
            anchors { left: parent.left; right: parent.right }

            Kirigami.Icon {
                id: appIcon
                source: "utilities-terminal"
                width: Kirigami.Units.iconSizes.medium
                height: Kirigami.Units.iconSizes.medium
            }

            PlasmaComponents3.Label {
                id: header
                text: i18nc("@title", "Konsole Profiles")
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                width: parent.width - appIcon.width * 2
                height: parent.height
            }
        }

        KSvg.SvgItem {
            id: separator

            anchors { left: headerRow.left; right: headerRow.right; top: headerRow.bottom }
            imagePath: "widgets/line"
            elementId: "horizontal-line"
        }

        Text {
            id: textMetric
            visible: false
            // translated but not used, we just need length/height
            text: i18n("Arbitrary String Which Says Something")
        }

        ScrollView {
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; top: separator.bottom; topMargin: Kirigami.Units.smallSpacing}

            ListView {
                id: view

                model: sortModel
                clip: true
                focus: true
                keyNavigationWraps: true

                delegate: PlasmaComponents3.ItemDelegate {
                    id: listdelegate

                    width: ListView.view.width
                    height: textMetric.paintedHeight * 2

                    hoverEnabled: true
                    text: model.name

                    Accessible.role: Accessible.Button

                    onClicked: {
                        openProfile();
                    }

                    onHoveredChanged: {
                        if (hovered) {
                            view.currentIndex = index;
                        }
                    }

                    function openProfile() {
                        /*var service = profilesSource.serviceForSource(model["DataEngineSource"])
                        var operation = service.operationDescription("open")
                        var  = service.startOperationCall(operation)*/
                        console.error(model.profileIdentifier)
                        profilesModel.openProfile(model.profileIdentifier)
                    }
                }

                highlight: PlasmaExtras.Highlight {}

                highlightMoveDuration: Kirigami.Units.longDuration
                highlightMoveVelocity: 1
            }
        }
    }
}
