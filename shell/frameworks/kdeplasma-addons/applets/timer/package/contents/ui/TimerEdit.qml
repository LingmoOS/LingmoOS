/*
 *  SPDX-FileCopyrightText: 2020 Łukasz Korbel <corebell.it@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.kquickcontrolsaddons 2.0 as QtExtra
import org.kde.plasma.private.timer 0.1 as TimerPlasmoid

Row {
    id: timerEdit
    property int value // time in seconds
    property bool editable: true
    // in alert mode 2nd set of digits from svg file will be drawn
    property bool alertMode: false

    signal digitModified(int valueDelta)

    QtObject {
        id: internal
        readonly property string digitSuffix: alertMode ? "_1" : ""
        // digits count include separators with 50% of digit width
        readonly property real digits: root.showSeconds ? 7 : 4.5;
        readonly property int digitH: (parent.height / 2) * digits < parent.width ? parent.height : parent.width / digits * 2
        readonly property int digitW: digitH / 2;
        property string valueString: "000000"
    }

    Component {
        id: digit
        KSvg.SvgItem {
            property int meaning
            property int num
            readonly property int mouseWheelAngleThreshold: 5
            width: internal.digitW
            height: internal.digitH

            activeFocusOnTab: true
            imagePath: "widgets/timer"
            elementId: num + internal.digitSuffix

            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Return:
                case Qt.Key_Enter:
                case Qt.Key_Space:
                case Qt.Key_Select:
                    root.toggleTimer();
                    break;
                case Qt.Key_Up:
                    if (value + meaning < 24*60*60) {
                        timerEdit.digitModified(meaning)
                    }
                    break;
                case Qt.Key_Down:
                    if (value - meaning >= 0) {
                        timerEdit.digitModified(-meaning)
                    }
                    break;
                case Qt.Key_Left:
                    nextItemInFocusChain(false).forceActiveFocus(Qt.BacktabFocusReason);
                    break;
                case Qt.Key_Right:
                    nextItemInFocusChain(true).forceActiveFocus(Qt.TabFocusReason);
                    break;
                default:
                    return;
                }
                event.accepted = true;
            }

            MouseArea {
                anchors.fill: parent
                enabled: editable
                propagateComposedEvents: true

                onWheel: wheel => {
                    wheel.accepted = true
                    if (wheel.angleDelta.y > mouseWheelAngleThreshold) {
                        if (value + meaning < 24*60*60) {
                            timerEdit.digitModified(meaning)
                        }
                    } else if (wheel.angleDelta.y < -mouseWheelAngleThreshold) {
                        if (value - meaning >= 0) {
                            timerEdit.digitModified(-meaning)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: separator
        KSvg.SvgItem {
            width: internal.digitW / 2;
            height: internal.digitH;
            imagePath: "widgets/timer"
            elementId: "separator" + internal.digitSuffix
        }
    }

    // Following 8 loaders will prepare all elements
    // to display time in format "hh:mm:ss"

    Loader {
        id: hour1
        sourceComponent: digit
        onLoaded: {
            item.meaning = 60*60*10 //10h
            item.num = internal.valueString[0]
        }
    }
    Loader {
        id: hour2
        sourceComponent: digit
        onLoaded: {
            item.meaning = 60*60 //1h
            item.num = internal.valueString[1]
        }
    }

    Loader { sourceComponent: separator } // ":"

    Loader {
        id: minute1
        sourceComponent: digit
        onLoaded: {
            item.meaning = 600 //10min
            item.num = internal.valueString[2]
        }
    }
    Loader {
        id: minute2
        sourceComponent: digit
        onLoaded: {
            item.meaning = 60 //1min
            item.num = internal.valueString[3]
        }
    }

    Loader { sourceComponent: root.showSeconds ? separator : undefined } // ":"

    Loader {
        id: second1
        sourceComponent: root.showSeconds ? digit : undefined
        onLoaded: {
            item.meaning = 10 //10s
            item.num = internal.valueString[4]
        }
    }
    Loader {
        id: second2
        sourceComponent: root.showSeconds ? digit : undefined
        onLoaded: {
            item.meaning = 1 //1s
            item.num = internal.valueString[5]
        }
    }

    onValueChanged: {
        // update all 6 digits in one signal handler
        internal.valueString = TimerPlasmoid.Timer.secondsToString(value, "hhmmss")
        if (hour1.item === null) return
        hour1.item.num = internal.valueString[0]
        hour2.item.num = internal.valueString[1]
        minute1.item.num = internal.valueString[2]
        minute2.item.num = internal.valueString[3]
        second1.item.num = internal.valueString[4]
        second2.item.num = internal.valueString[5]
    }
}

