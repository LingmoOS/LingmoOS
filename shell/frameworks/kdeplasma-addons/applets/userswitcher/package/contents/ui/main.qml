/*
 *  SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

import org.kde.coreaddons 1.0 as KCoreAddons // kuser
import org.kde.kirigami 2.20 as Kirigami
import org.kde.config as KConfig  // KAuthorized.authorizeControlModule
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kirigamiaddons.components 1.0 as KirigamiComponents
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.sessions 2.0 as Sessions

PlasmoidItem {
    id: root

    readonly property bool isVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
        || Plasmoid.location === PlasmaCore.Types.RightEdge
        || Plasmoid.location === PlasmaCore.Types.BottomEdge
        || Plasmoid.location === PlasmaCore.Types.LeftEdge)


    readonly property string displayedName: showFullName ? kuser.fullName : kuser.loginName

    readonly property bool showFace: Plasmoid.configuration.showFace
    readonly property bool showName: Plasmoid.configuration.showName
    readonly property string avatarIcon: kuser.faceIconUrl.toString()

    readonly property bool showFullName: Plasmoid.configuration.showFullName

    // TTY number and X display
    readonly property bool showTechnicalInfo: Plasmoid.configuration.showTechnicalInfo

    switchWidth: Kirigami.Units.gridUnit * 10
    switchHeight: Kirigami.Units.gridUnit * 12

    toolTipTextFormat: Text.StyledText
    toolTipSubText: i18n("You are logged in as <b>%1</b>", displayedName)

    // revert to the Plasmoid icon if no face given
    Plasmoid.icon: kuser.faceIconUrl.toString() || (inPanel ? "system-switch-user-symbolic" : "preferences-system-users" )

    KCoreAddons.KUser {
        id: kuser
    }

    compactRepresentation: MouseArea {
        id: compactRoot

        // Taken from DigitalClock to ensure uniform sizing when next to each other
        readonly property bool tooSmall: Plasmoid.formFactor === PlasmaCore.Types.Horizontal && Math.round(2 * (compactRoot.height / 5)) <= Kirigami.Theme.smallFont.pixelSize

        Layout.minimumWidth: isVertical ? 0 : compactRow.implicitWidth
        Layout.maximumWidth: isVertical ? Infinity : Layout.minimumWidth
        Layout.preferredWidth: isVertical ? -1 : Layout.minimumWidth

        Layout.minimumHeight: isVertical ? label.height : Kirigami.Theme.smallFont.pixelSize
        Layout.maximumHeight: isVertical ? Layout.minimumHeight : Infinity
        Layout.preferredHeight: isVertical ? Layout.minimumHeight : Kirigami.Units.iconSizes.sizeForLabels * 2

        property bool wasExpanded
        onPressed: wasExpanded = root.expanded
        onClicked: root.expanded = !wasExpanded

        Row {
            id: compactRow

            anchors.centerIn: parent
            spacing: Kirigami.Units.smallSpacing

            KirigamiComponents.Avatar {
                id: icon

                anchors.verticalCenter: parent.verticalCenter
                height: compactRoot.height - Math.round(Kirigami.Units.smallSpacing / 2)
                width: height

                name: root.displayedName

                source: visible ? root.avatarIcon : ""
                visible: root.showFace
            }

            PlasmaComponents3.Label {
                id: label

                width: root.isVertical ? compactRoot.width : contentWidth
                height: root.isVertical ? contentHeight : compactRoot.height

                text: root.displayedName
                textFormat: Text.PlainText
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.NoWrap
                fontSizeMode: root.isVertical ? Text.HorizontalFit : Text.VerticalFit
                font.pixelSize: tooSmall ? Kirigami.Theme.defaultFont.pixelSize : Kirigami.Units.iconSizes.roundedIconSize(Kirigami.Units.gridUnit * 2)
                minimumPointSize: Kirigami.Theme.smallFont.pointSize
                visible: root.showName
            }
        }
    }

    fullRepresentation: Item {
        id: fullRoot

        implicitHeight: column.implicitHeight
        implicitWidth: column.implicitWidth

        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        Layout.preferredHeight: implicitHeight
        Layout.minimumWidth: Layout.preferredWidth
        Layout.minimumHeight: Layout.preferredHeight
        Layout.maximumWidth: Layout.preferredWidth
        Layout.maximumHeight: Screen.height / 2

        Sessions.SessionManagement {
            id: sm
        }

        Sessions.SessionsModel {
            id: sessionsModel
        }

        ColumnLayout {
            id: column

            anchors.fill: parent
            spacing: 0

            UserListDelegate {
                id: currentUserItem
                text: root.displayedName
                subText: i18n("Current user")
                source: root.avatarIcon
                hoverEnabled: false
            }

            PlasmaComponents3.ScrollView {
                id: scroll

                Layout.fillWidth: true
                Layout.fillHeight: true

                // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
                PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

                ListView {
                    id: userList
                    model: sessionsModel

                    focus: true
                    interactive: true
                    keyNavigationWraps: true

                    delegate: UserListDelegate {
                        width: ListView.view.width

                        activeFocusOnTab: true

                        text: {
                            if (!model.session) {
                                return i18nc("Nobody logged in on that session", "Unused")
                            }

                            if (model.realName && root.showFullName) {
                                return model.realName
                            }

                            return model.name
                        }
                        source: model.icon
                        subText: {
                            if (!root.showTechnicalInfo) {
                                return ""
                            }

                            if (model.isTty) {
                                return i18nc("User logged in on console number", "TTY %1", model.vtNumber)
                            } else if (model.displayNumber) {
                                return i18nc("User logged in on console (X display number)", "on %1 (%2)", model.vtNumber, model.displayNumber)
                            }
                            return ""
                        }

                        KeyNavigation.up: index === 0 ? currentUserItem.nextItemInFocusChain() : userList.itemAtIndex(index - 1)
                        KeyNavigation.down: index === userList.count - 1 ? newSessionButton : userList.itemAtIndex(index + 1)

                        Accessible.description: i18nc("@action:button", "Switch to User %1", text)

                        onClicked: sessionsModel.switchUser(model.vtNumber, sessionsModel.shouldLock)
                    }
                }
            }

            ActionListDelegate {
                id: newSessionButton
                text: i18nc("@action", "New Session")
                icon.name: "system-switch-user"
                visible: sessionsModel.canStartNewSession

                KeyNavigation.up: userList.count > 0 ? userList.itemAtIndex(userList.count - 1) : currentUserItem.nextItemInFocusChain()
                KeyNavigation.down: lockScreenButton

                onClicked: sessionsModel.startNewSession(sessionsModel.shouldLock)
            }

            ActionListDelegate {
                id: lockScreenButton
                text: i18nc("@action", "Lock Screen")
                icon.name: "system-lock-screen"
                visible: sm.canLock

                KeyNavigation.up: newSessionButton
                KeyNavigation.down: leaveButton

                onClicked: sm.lock()
            }

            ActionListDelegate {
                id: leaveButton
                text: i18nc("Show a dialog with options to logout/shutdown/restart", "Log Out")
                icon.name: "system-log-out"
                visible: sm.canLogout

                KeyNavigation.up: lockScreenButton

                onClicked: sm.requestLogout()
            }
        }

        Connections {
            target: root
            function onExpandedChanged() {
                if (root.expanded) {
                    sessionsModel.reload();
                }
            }
        }
    }
}
