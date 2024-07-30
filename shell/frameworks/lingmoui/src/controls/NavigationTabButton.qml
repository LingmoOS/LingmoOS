/* SPDX-FileCopyrightText: 2021 Devin Lin <espidev@gmail.com>
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI

/**
 * @brief Navigation buttons to be used for the NavigationTabBar component.
 *
 * It supplies its own padding, and also supports using the QQC2 AbstractButton ``display`` property to be used in column lists.
 *
 * Alternative way to the "actions" property on NavigationTabBar, as it can be used
 * with Repeater to generate buttons from models.
 *
 * Example usage:
 * @code{.qml}
 * LingmoUI.NavigationTabBar {
 *      id: navTabBar
 *      LingmoUI.NavigationTabButton {
 *          visible: true
 *          icon.name: "document-save"
 *          text: `test ${tabIndex + 1}`
 *          QQC2.ButtonGroup.group: navTabBar.tabGroup
 *      }
 *      LingmoUI.NavigationTabButton {
 *          visible: false
 *          icon.name: "document-send"
 *          text: `test ${tabIndex + 1}`
 *          QQC2.ButtonGroup.group: navTabBar.tabGroup
 *      }
 *      actions: [
 *          LingmoUI.Action {
 *              visible: true
 *              icon.name: "edit-copy"
 *              icon.height: 32
 *              icon.width: 32
 *              text: `test 3`
 *              checked: true
 *          },
 *          LingmoUI.Action {
 *              visible: true
 *              icon.name: "edit-cut"
 *              text: `test 4`
 *              checkable: true
 *          },
 *          LingmoUI.Action {
 *              visible: false
 *              icon.name: "edit-paste"
 *              text: `test 5`
 *          },
 *          LingmoUI.Action {
 *              visible: true
 *              icon.source: "../logo.png"
 *              text: `test 6`
 *              checkable: true
 *          }
 *      ]
 *  }
 * @endcode
 *
 * @since 5.87
 * @since org.kde.lingmoui 2.19
 * @inherit QtQuick.Templates.TabButton
 */
T.TabButton {
    id: control

    /**
     * @brief This property tells the index of this tab within the tab bar.
     */
    readonly property int tabIndex: {
        let tabIdx = 0
        for (const child of parent.children) {
            if (child === this) {
                return tabIdx
            }
            // Checking for AbstractButtons because any AbstractButton can act as a tab
            if (child instanceof T.AbstractButton) {
                ++tabIdx
            }
        }
        return -1
    }

    // FIXME: all those internal properties should go, and the button should style itself in a more standard way
    // probably similar to view items
    readonly property color __foregroundColor: LingmoUI.Theme.textColor
    readonly property color __highlightForegroundColor: LingmoUI.Theme.textColor

    readonly property color __pressedColor: Qt.alpha(LingmoUI.Theme.highlightColor, 0.3)
    readonly property color __hoverSelectColor: Qt.alpha(LingmoUI.Theme.highlightColor, 0.2)
    readonly property color __checkedBorderColor: Qt.alpha(LingmoUI.Theme.highlightColor, 0.7)
    readonly property color __pressedBorderColor: Qt.alpha(LingmoUI.Theme.highlightColor, 0.9)

    readonly property real __verticalMargins: (display === T.AbstractButton.TextBesideIcon) ? LingmoUI.Units.largeSpacing : 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    display: T.AbstractButton.TextUnderIcon

    LingmoUI.Theme.colorSet: LingmoUI.Theme.Window
    LingmoUI.Theme.inherit: false

    hoverEnabled: true

    padding: LingmoUI.Units.smallSpacing
    spacing: LingmoUI.Units.smallSpacing

    icon.height: display === T.AbstractButton.TextBesideIcon ? LingmoUI.Units.iconSizes.small : LingmoUI.Units.iconSizes.smallMedium
    icon.width: display === T.AbstractButton.TextBesideIcon ? LingmoUI.Units.iconSizes.small : LingmoUI.Units.iconSizes.smallMedium
    icon.color: checked ? __highlightForegroundColor : __foregroundColor

    LingmoUI.MnemonicData.enabled: enabled && visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.MenuItem
    LingmoUI.MnemonicData.label: text

    Accessible.onPressAction: control.action.trigger()

    background: Rectangle {
        LingmoUI.Theme.colorSet: LingmoUI.Theme.Button
        LingmoUI.Theme.inherit: false

        implicitHeight: (control.display === T.AbstractButton.TextBesideIcon) ? 0 : (LingmoUI.Units.gridUnit * 3 + LingmoUI.Units.smallSpacing * 2)

        color: "transparent"

        Rectangle {
            width: parent.width - LingmoUI.Units.largeSpacing
            height: parent.height - LingmoUI.Units.largeSpacing
            anchors.centerIn: parent

            radius: LingmoUI.Units.cornerRadius
            color: control.down ? control.__pressedColor : (control.checked || control.hovered ? control.__hoverSelectColor : "transparent")

            border.color: control.checked ? control.__checkedBorderColor : (control.down ? control.__pressedBorderColor : color)
            border.width: 1

            Behavior on color { ColorAnimation { duration: LingmoUI.Units.shortDuration } }
            Behavior on border.color { ColorAnimation { duration: LingmoUI.Units.shortDuration } }
        }
    }

    contentItem: GridLayout {
        columnSpacing: 0
        rowSpacing: (label.visible && label.lineCount > 1) ? 0 : control.spacing

        // if this is a row or a column
        columns: control.display !== T.AbstractButton.TextBesideIcon ? 1 : 2

        LingmoUI.Icon {
            id: icon
            source: control.icon.name || control.icon.source
            visible: (control.icon.name.length > 0 || control.icon.source.toString().length > 0) && control.display !== T.AbstractButton.TextOnly
            color: control.icon.color

            Layout.topMargin: control.__verticalMargins
            Layout.bottomMargin: control.__verticalMargins
            Layout.leftMargin: (control.display === T.AbstractButton.TextBesideIcon) ? LingmoUI.Units.gridUnit : 0
            Layout.rightMargin: (control.display === T.AbstractButton.TextBesideIcon) ? LingmoUI.Units.gridUnit : 0

            Layout.alignment: {
                if (control.display === T.AbstractButton.TextBesideIcon) {
                    // row layout
                    return Qt.AlignVCenter | Qt.AlignRight;
                } else {
                    // column layout
                    return Qt.AlignHCenter | ((!label.visible || label.lineCount > 1) ? Qt.AlignVCenter : Qt.AlignBottom);
                }
            }
            implicitHeight: source ? control.icon.height : 0
            implicitWidth: source ? control.icon.width : 0

            Behavior on color { ColorAnimation { duration: LingmoUI.Units.shortDuration } }
        }
        QQC2.Label {
            id: label

            text: control.LingmoUI.MnemonicData.richTextLabel
            horizontalAlignment: (control.display === T.AbstractButton.TextBesideIcon) ? Text.AlignLeft : Text.AlignHCenter

            visible: control.display !== T.AbstractButton.IconOnly
            wrapMode: Text.Wrap
            elide: Text.ElideMiddle
            color: control.checked ? control.__highlightForegroundColor : control.__foregroundColor

            font.bold: control.checked
            font.pointSize: !icon.visible && control.display === T.AbstractButton.TextBelowIcon
                    ? LingmoUI.Theme.defaultFont.pointSize * 1.20 // 1.20 is equivalent to level 2 heading
                    : LingmoUI.Theme.defaultFont.pointSize

            Behavior on color { ColorAnimation { duration: LingmoUI.Units.shortDuration } }

            Layout.topMargin: control.__verticalMargins
            Layout.bottomMargin: control.__verticalMargins

            Layout.alignment: {
                if (control.display === T.AbstractButton.TextBesideIcon) {
                    // row layout
                    return Qt.AlignVCenter | Qt.AlignLeft;
                } else {
                    // column layout
                    return icon.visible ? Qt.AlignHCenter | Qt.AlignTop : Qt.AlignCenter;
                }
            }

            Layout.fillWidth: true

            Accessible.ignored: true
        }
    }
}
