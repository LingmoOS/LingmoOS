/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean
import org.kde.ocean.impl as Impl

T.ToolButton {
    id: control

    // HACK: Compatibility with qqc2-desktop-style hack for showing arrows when buttons open menus
    // This one is at the level of `control` to make it more reliably accessible to the indicator.
    // Unlike qqc2-desktop-style, the arrow is in the indicator property instead of the background.
    property bool __showMenuArrow: false

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    flat: true

    hoverEnabled: Qt.styleHints.useHoverEffects

    LingmoUI.Theme.colorSet: /*control.highlighted ? LingmoUI.Theme.Selection :*/ LingmoUI.Theme.Button
    LingmoUI.Theme.inherit: false//control.flat && !control.down && !control.checked
    // Absolutely terrible HACK:
    // For some reason, ActionToolBar overrides the colorSet and inherit attached properties
    Component.onCompleted: {
        //console.log("colorSet: " + LingmoUI.Theme.colorSet)
        //console.log("inherit: " + LingmoUI.Theme.inherit)
        LingmoUI.Theme.colorSet = LingmoUI.Theme.Button/*Qt.binding(() => control.highlighted ? LingmoUI.Theme.Selection : LingmoUI.Theme.Button)*/
        LingmoUI.Theme.inherit = false//Qt.binding(() => control.flat && !(control.down || control.checked))
    }

    padding: LingmoUI.Units.mediumSpacing
    leftPadding: {
        if ((!contentItem.hasIcon && contentItem.textBesideIcon) // False if contentItem has been replaced
            || display == T.AbstractButton.TextOnly
            || display == T.AbstractButton.TextUnderIcon) {
            return Impl.Units.largeHorizontalPadding
        } else {
            return control.horizontalPadding
        }
    }
    rightPadding: {
        if (contentItem.hasLabel && display != T.AbstractButton.IconOnly) { // False if contentItem has been replaced
            return Impl.Units.largeHorizontalPadding
        } else {
            return control.horizontalPadding
        }
    }

    spacing: LingmoUI.Units.mediumSpacing

    icon.width: LingmoUI.Units.iconSizes.sizeForLabels
    icon.height: LingmoUI.Units.iconSizes.sizeForLabels

    LingmoUI.MnemonicData.enabled: control.enabled && control.visible
    LingmoUI.MnemonicData.controlType: LingmoUI.MnemonicData.ActionElement
    LingmoUI.MnemonicData.label: control.display !== T.Button.IconOnly ? control.text : ""
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.LingmoUI.MnemonicData.sequence
        onActivated: control.clicked()
    }

    contentItem:Impl.IconLabelContent {
        control: control
        text: control.LingmoUI.MnemonicData.richTextLabel
    }

    // Using a Loader here reduces the RAM usage
    indicator: Loader {
        property alias iconHeight: control.icon.height
        property alias iconWidth: control.icon.width
        anchors {
            right: control.right
            rightMargin: control.rightPadding
            verticalCenter: control.verticalCenter
        }
        visible: control.__showMenuArrow
        active: visible
        sourceComponent: Component {
            LingmoUI.Icon {
                anchors.centerIn: parent
                implicitHeight: iconHeight
                implicitWidth: iconWidth
                source: "arrow-down"
            }
        }
    }

    background: Impl.ButtonBackground {
        // HACK: Compatibility with qqc2-desktop-style hack for showing arrows when buttons open menus
        // This one is in the background because that's what LingmoUI expects
        property alias showMenuArrow: control.__showMenuArrow
        control: control
    }
}
