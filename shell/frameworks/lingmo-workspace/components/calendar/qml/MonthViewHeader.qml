/*
    SPDX-FileCopyrightText: 2022 Tanbir Jishan <tantalising007@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.lingmoui as LingmoUI
import org.kde.lingmo.components as LingmoComponents
import org.kde.lingmo.extras as LingmoExtras

// NOTE : This header is designed to be usable by both the generic calendar component and the digital clock applet
// which requires a little different layout to accomodate for configure and pin buttons because it may be in panel

//                      CALENDAR                                               DIGTAL CLOCK
// |---------------------------------------------------|  |----------------------------------------------------|
// | January                              < today >    |  | January                            config+  pin/   |
// |        Days          Months         Year          |  |   Days       Months       Year          < today >  |
// |                                                   |  |                                                    |
// |               Rest of the calendar                |  |              Rest of the calendar                  |
// |...................................................|  |....................................................|
//


Item {
    id: root

    required property T.SwipeView swipeView
    // Can't use the real type due to a "Cyclic dependency"
    required property QtObject /*MonthView*/ monthViewRoot

    readonly property bool isDigitalClock: monthViewRoot.showDigitalClockHeader
    readonly property var buttons: isDigitalClock ? dateManipulationButtonsForDigitalClock : dateManipulationButtons
    readonly property T.AbstractButton tabButton: isDigitalClock ? configureButton : todayButton
    readonly property T.AbstractButton previousButton: buttons.previousButton
    readonly property T.AbstractButton todayButton: buttons.todayButton
    readonly property T.AbstractButton nextButton: buttons.nextButton
    readonly property alias tabBar: tabBar
    readonly property alias heading: heading
    readonly property alias configureButton: dateAndPinButtons.configureButton
    readonly property alias pinButton: dateAndPinButtons.pinButton

    implicitWidth: viewHeader.implicitWidth
    implicitHeight: viewHeader.implicitHeight

    KeyNavigation.up: configureButton

    Loader {
        anchors.fill: parent
        sourceComponent: LingmoExtras.PlasmoidHeading {}
        active: root.isDigitalClock
    }

    ColumnLayout {
        id: viewHeader
        width: parent.width
        spacing: LingmoUI.Units.smallSpacing

        RowLayout {
            spacing: 0
            Layout.leftMargin: LingmoUI.Units.largeSpacing

            LingmoUI.Heading {
                id: heading
                // Needed for Appium testing
                objectName: "monthHeader"

                text: root.swipeView.currentIndex > 0 || root.monthViewRoot.selectedYear !== today.getFullYear()
                    ? i18ndc("lingmoshellprivateplugin", "Format: month year", "%1 %2",
                        root.monthViewRoot.selectedMonth, root.monthViewRoot.selectedYear.toString())
                    : root.monthViewRoot.selectedMonth
                textFormat: Text.PlainText
                level: root.isDigitalClock ? 1 : 2
                elide: Text.ElideRight
                font.capitalization: Font.Capitalize
                Layout.fillWidth: true
            }

            Loader {
                id: dateManipulationButtons

                readonly property T.AbstractButton previousButton: item?.previousButton ?? null
                readonly property T.AbstractButton todayButton: item?.todayButton ?? null
                readonly property T.AbstractButton nextButton: item?.nextButton ?? null

                sourceComponent: buttonsGroup
                active: !root.isDigitalClock
            }

            Loader {
                id: dateAndPinButtons

                readonly property T.AbstractButton configureButton: item?.configureButton ?? null
                readonly property T.AbstractButton pinButton: item?.pinButton ?? null

                sourceComponent: dateAndPin
                active: root.isDigitalClock
            }
        }

        RowLayout {
            spacing: 0
            LingmoComponents.TabBar {
                id: tabBar

                currentIndex: root.swipeView.currentIndex
                Layout.fillWidth: true
                Layout.bottomMargin: root.isDigitalClock ? 0 : LingmoUI.Units.smallSpacing

                KeyNavigation.up: root.isDigitalClock ? root.configureButton : root.previousButton
                KeyNavigation.right: dateManipulationButtonsForDigitalClock.previousButton
                KeyNavigation.left: root.monthViewRoot.eventButton && root.monthViewRoot.eventButton.visible
                    ? root.monthViewRoot.eventButton
                    : root.monthViewRoot.eventButton && root.monthViewRoot.eventButton.KeyNavigation.down

                onFocusChanged: {
                    if (!focus) {
                        // Rebind after potential key navigation
                        currentIndex = Qt.binding(() => root.swipeView.currentIndex);
                    }
                }

                LingmoComponents.TabButton {
                    Accessible.onPressAction: clicked()
                    text: i18nd("lingmoshellprivateplugin", "Days");
                    onClicked: root.monthViewRoot.showMonthView();
                    display: T.AbstractButton.TextOnly
                }
                LingmoComponents.TabButton {
                    Accessible.onPressAction: clicked()
                    text: i18nd("lingmoshellprivateplugin", "Months");
                    onClicked: root.monthViewRoot.showYearView();
                    display: T.AbstractButton.TextOnly
                }
                LingmoComponents.TabButton {
                    Accessible.onPressAction: clicked()
                    text: i18nd("lingmoshellprivateplugin", "Years");
                    onClicked: root.monthViewRoot.showDecadeView();
                    display: T.AbstractButton.TextOnly
                }
            }

            Loader {
                id: dateManipulationButtonsForDigitalClock

                readonly property T.AbstractButton previousButton: item?.previousButton ?? null
                readonly property T.AbstractButton todayButton: item?.todayButton ?? null
                readonly property T.AbstractButton nextButton: item?.nextButton ?? null

                sourceComponent: buttonsGroup
                active: root.isDigitalClock
            }
        }
    }

    // ------------------------------------------ UI ends ------------------------------------------------- //

    Component {
        id: buttonsGroup

        RowLayout {
            spacing: 0

            readonly property alias previousButton: previousButton
            readonly property alias todayButton: todayButton
            readonly property alias nextButton: nextButton

            KeyNavigation.up: root.configureButton

            LingmoComponents.ToolButton {
                id: previousButton
                text: {
                    switch (root.monthViewRoot.calendarViewDisplayed) {
                    case MonthView.CalendarView.DayView:
                        return i18nd("lingmoshellprivateplugin", "Previous Month")
                    case MonthView.CalendarView.MonthView:
                        return i18nd("lingmoshellprivateplugin", "Previous Year")
                    case MonthView.CalendarView.YearView:
                        return i18nd("lingmoshellprivateplugin", "Previous Decade")
                    default:
                        return "";
                    }
                }

                icon.name: Qt.application.layoutDirection === Qt.RightToLeft ? "go-next" : "go-previous"
                display: T.AbstractButton.IconOnly
                KeyNavigation.right: todayButton

                onClicked: root.monthViewRoot.previousView()

                LingmoComponents.ToolTip { text: parent.text }
            }

            LingmoComponents.ToolButton {
                id: todayButton
                text: i18ndc("lingmoshellprivateplugin", "Reset calendar to today", "Today")
                Accessible.description: i18nd("lingmoshellprivateplugin", "Reset calendar to today")
                KeyNavigation.right: nextButton

                onClicked: root.monthViewRoot.resetToToday()
            }

            LingmoComponents.ToolButton {
                id: nextButton
                text: {
                    switch (root.monthViewRoot.calendarViewDisplayed) {
                    case MonthView.CalendarView.DayView:
                        return i18nd("lingmoshellprivateplugin", "Next Month")
                    case MonthView.CalendarView.MonthView:
                        return i18nd("lingmoshellprivateplugin", "Next Year")
                    case MonthView.CalendarView.YearView:
                        return i18nd("lingmoshellprivateplugin", "Next Decade")
                    default:
                        return "";
                    }
                }

                icon.name: Qt.application.layoutDirection === Qt.RightToLeft ? "go-previous" : "go-next"
                display: T.AbstractButton.IconOnly
                KeyNavigation.tab: root.swipeView

                onClicked: root.monthViewRoot.nextView();

                LingmoComponents.ToolTip { text: parent.text }
            }
        }
    }

    Component {
        id: dateAndPin

        RowLayout {
            spacing: 0

            readonly property alias configureButton: configureButton
            readonly property alias pinButton: pinButton

            KeyNavigation.up: pinButton

            LingmoComponents.ToolButton {
                id: configureButton

                visible: root.monthViewRoot.digitalClock.internalAction("configure").enabled

                display: T.AbstractButton.IconOnly
                icon.name: "configure"
                text: root.monthViewRoot.digitalClock.internalAction("configure").text

                KeyNavigation.left: tabBar.KeyNavigation.left
                KeyNavigation.right: pinButton
                KeyNavigation.down: root.todayButton

                onClicked: root.monthViewRoot.digitalClock.internalAction("configure").trigger()
                LingmoComponents.ToolTip {
                    text: parent.text
                }
            }

            // Allows the user to keep the calendar open for reference
            LingmoComponents.ToolButton {
                id: pinButton

                checkable: true
                checked: root.monthViewRoot.digitalClock.configuration.pin

                display: T.AbstractButton.IconOnly
                icon.name: "window-pin"
                text: i18n("Keep Open")

                KeyNavigation.down: root.nextButton

                onToggled: root.monthViewRoot.digitalClock.configuration.pin = checked

                LingmoComponents.ToolTip {
                    text: parent.text
                }
            }
        }
    }
}
