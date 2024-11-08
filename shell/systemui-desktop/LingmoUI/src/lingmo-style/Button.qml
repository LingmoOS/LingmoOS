import QtQuick 2.12
import QtQuick.Templates 2.12 as T
import QtGraphicalEffects 1.0
import LingmoUI 1.0 as LingmoUI
import QtQuick.Controls.impl 2.12

T.Button {
    id: control
    implicitWidth: Math.max(background.implicitWidth, contentItem.implicitWidth + LingmoUI.Units.largeSpacing)
    implicitHeight: background.implicitHeight
    hoverEnabled: true

    icon.width: LingmoUI.Units.iconSizes.small
    icon.height: LingmoUI.Units.iconSizes.small

    icon.color: control.enabled ? (control.highlighted ? control.LingmoUI.Theme.highlightColor : control.LingmoUI.Theme.textColor) : control.LingmoUI.Theme.disabledTextColor
    spacing: LingmoUI.Units.smallSpacing

    // property color hoveredColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.alternateBackgroundColor, 1.2)
    //                                                    : Qt.darker(LingmoUI.Theme.alternateBackgroundColor, 1.1)

    // property color pressedColor: LingmoUI.Theme.darkMode ? Qt.lighter(LingmoUI.Theme.alternateBackgroundColor, 1.1)
    //                                                    : Qt.darker(LingmoUI.Theme.alternateBackgroundColor, 1.2)

    property color hoveredColor: Qt.tint(LingmoUI.Theme.textColor, Qt.rgba(LingmoUI.Theme.backgroundColor.r,
                                                                       LingmoUI.Theme.backgroundColor.g,
                                                                       LingmoUI.Theme.backgroundColor.b, 0.9))
    property color pressedColor: Qt.tint(LingmoUI.Theme.textColor, Qt.rgba(LingmoUI.Theme.backgroundColor.r,
                                                                       LingmoUI.Theme.backgroundColor.g,
                                                                       LingmoUI.Theme.backgroundColor.b, 0.8))


    // property color borderColor: Qt.rgba(LingmoUI.Theme.highlightColor.r,
    //                                     LingmoUI.Theme.highlightColor.g,
    //                                     LingmoUI.Theme.highlightColor.b, 0.5)

    // property color flatHoveredColor: Qt.rgba(LingmoUI.Theme.highlightColor.r,
    //                                          LingmoUI.Theme.highlightColor.g,
    //                                          LingmoUI.Theme.highlightColor.b, 0.2)
    // property color flatPressedColor: Qt.rgba(LingmoUI.Theme.highlightColor.r,
    //                                          LingmoUI.Theme.highlightColor.g,
    //                                          LingmoUI.Theme.highlightColor.b, 0.25)

    property color flatHoveredColor: Qt.lighter(LingmoUI.Theme.highlightColor, 1.1)
    property color flatPressedColor: Qt.darker(LingmoUI.Theme.highlightColor, 1.1)

    contentItem: IconLabel {
        text: control.text
        font: control.font
        icon: control.icon
        color: !control.enabled ? control.LingmoUI.Theme.disabledTextColor : control.flat ? LingmoUI.Theme.highlightedTextColor : LingmoUI.Theme.textColor
        spacing: control.spacing
        mirrored: control.mirrored
        display: control.display
        alignment: Qt.AlignCenter
    }

    background: Item {
        implicitWidth: (LingmoUI.Units.iconSizes.medium * 3) + LingmoUI.Units.largeSpacing
        implicitHeight: LingmoUI.Units.iconSizes.medium + LingmoUI.Units.smallSpacing

        Rectangle {
            id: _flatBackground
            anchors.fill: parent
            radius: LingmoUI.Theme.mediumRadius
            border.width: 1
            border.color: control.enabled ? control.activeFocus ? LingmoUI.Theme.highlightColor : "transparent"
                                          : "transparent"
            visible: control.flat


            color: {
                if (!control.enabled)
                    return LingmoUI.Theme.alternateBackgroundColor

                if (control.pressed)
                    return control.flatPressedColor

                if (control.hovered)
                    return control.flatHoveredColor

                return Qt.rgba(LingmoUI.Theme.highlightColor.r,
                               LingmoUI.Theme.highlightColor.g,
                               LingmoUI.Theme.highlightColor.b, 0.1)
            }

            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0; color: Qt.rgba(_flatBackground.color.r,
                                                             _flatBackground.color.g,
                                                             _flatBackground.color.b, 0.85) }
                GradientStop { position: 1.0; color: Qt.rgba(_flatBackground.color.r,
                                                             _flatBackground.color.g,
                                                             _flatBackground.color.b, 1) }
            }

            // Behavior on color {
            //    ColorAnimation {
            //        duration: 200
            //        easing.type: Easing.Linear
            //    }
            // }
        }

        Rectangle {
            id: _background
            anchors.fill: parent
            radius: LingmoUI.Theme.hugeRadius
            border.width: 1
            visible: !control.flat
            border.color: control.enabled ? control.activeFocus ? LingmoUI.Theme.highlightColor : "transparent"
                                          : "transparent"

            color: {
                if (!control.enabled)
                    return LingmoUI.Theme.alternateBackgroundColor

                if (control.pressed)
                    return control.pressedColor

                if (control.hovered)
                    return control.hoveredColor

                return LingmoUI.Theme.alternateBackgroundColor
            }
        }
    }
}
