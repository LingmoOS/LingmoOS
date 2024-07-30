/*
 *   SPDX-FileCopyrightText: 2008, 2014 Davide Bettio <davide.bettio@kdemail.net>
 *   SPDX-FileCopyrightText: 2015 Bernhard Friedreich <friesoft@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.lingmo.plasmoid 2.0
import org.kde.lingmoui 2.20 as LingmoUI
import org.kde.lingmo.components 3.0 as LingmoComponents3
import org.kde.lingmo.extras 2.0 as LingmoExtras
import org.kde.kquickcontrolsaddons 2.0 as QtExtra

MouseArea {
    Layout.preferredWidth: Math.max(Plasmoid.compactRepresentationItem.width, LingmoUI.Units.gridUnit * 10)
    Layout.preferredHeight: main.implicitHeight

    onClicked: root.toggleTimer()

    Component {
        id: popupHeadingComponent

        LingmoExtras.PlasmoidHeading {
            leftPadding: LingmoUI.Units.smallSpacing * 2
            rightPadding: LingmoUI.Units.smallSpacing * 2

            contentItem: LingmoUI.Heading {
                level: 3
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                text: root.title
                textFormat: Text.PlainText
            }
        }
    }

    Component {
        id: desktopHeadingComponent

        LingmoComponents3.Label {
            elide: Text.ElideRight
            font.pixelSize: 0.3 * timerDigits.height
            text: root.title
            textFormat: Text.PlainText
        }
    }

    ColumnLayout {
        id: main

        width: parent.width

        Loader {
            Layout.fillWidth: true

            active: root.showTitle

            sourceComponent: root.inPanel ? popupHeadingComponent : desktopHeadingComponent
        }

        TimerEdit {
            id: timerDigits

            Layout.fillWidth: true
            visible: root.showRemainingTime

            value: root.seconds
            editable: !root.running
            alertMode: root.alertMode
            onDigitModified: valueDelta => root.seconds += valueDelta
            SequentialAnimation on opacity {
                running: root.suspended;
                loops: Animation.Infinite;
                NumberAnimation {
                    duration: LingmoUI.Units.veryLongDuration * 2;
                    from: 1.0;
                    to: 0.2;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: LingmoUI.Units.veryLongDuration;
                }
                NumberAnimation {
                    duration: LingmoUI.Units.veryLongDuration * 2;
                    from: 0.2;
                    to: 1.0;
                    easing.type: Easing.InOutQuad;
                }
                PauseAnimation {
                    duration: LingmoUI.Units.veryLongDuration;
                }
            }
        }

        LingmoComponents3.ProgressBar {
            id: remainingTimeProgressBar

            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.showProgressBar

            from: plasmoid.configuration.seconds
            to: 0
            value: root.seconds
        }

        function resetOpacity() {
            timerDigits.opacity = 1.0;
        }

        Component.onCompleted: {
            root.opacityNeedsReset.connect(resetOpacity);
        }
    }
}

