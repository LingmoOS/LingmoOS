// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import QtQuick 2.0
import org.deepin.dtk 1.0 as D

QtObject {
    property QtObject control: QtObject {
        property int radius: 8
        property int spacing: 6
        property int padding: 6
        property int borderWidth: 1
        property real focusBorderWidth: 2
        property real focusBorderPaddings: 1

        property D.Palette border: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.05)
            normalDark: Qt.rgba(0, 0, 0, 0.05)
        }

        function implicitWidth(control) {
            return Math.max(backgroundImplicitWidth(control), contentImplicitWidth(control))
        }
        function implicitHeight(control) {
            var background = control.implicitBackgroundHeight !== undefined
                    ? control.implicitBackgroundHeight
                    : (control.background ? control.background.implicitHeight : 0)

            if (control.topInset !== undefined)
                background += (control.topInset + control.bottomInset)

            var content = control.implicitContentHeight !== undefined
                    ? control.implicitContentHeight
                    : (control.contentItem ? control.contentItem.implicitHeight : 0)
            content += (control.topPadding + control.bottomPadding)

            return Math.max(background, content)
        }
        function backgroundImplicitWidth(control) {
            var background = control.implicitBackgroundWidth !== undefined
                    ? control.implicitBackgroundWidth
                    : (control.background ? control.background.implicitWidth : 0)

            if (control.leftInset !== undefined)
                background += (control.leftInset + control.rightInset)

            return background
        }
        function contentImplicitWidth(control) {
            var content = control.implicitContentWidth !== undefined
                    ? control.implicitContentWidth
                    : (control.contentItem ? control.contentItem.implicitWidth : 0)
            content += (control.leftPadding + control.rightPadding)

            return content
        }
        function selectColor(conditionColor, lightColor, darkColor) {
            if (conditionColor) {
                return D.DTK.selectColor(conditionColor, lightColor, darkColor)
            }

            return D.DTK.themeType === D.ApplicationHelper.DarkType ? darkColor  : lightColor
        }
    }

    // Settings
    property QtObject settings: QtObject {
        property QtObject title: QtObject {
            property int marginL1: 10
            property int marginL2: 30
            property int marginLOther: 50
        }

        property QtObject content: QtObject {
            property int margin: 10
            property int marginL1: 10
            property int marginL2: 30
            property int marginOther: 50
            property int resetButtonHeight: 90
        }

        property QtObject navigation: QtObject {
            property int width: 190
            property int height: 20
            property int margin: 10
            property int textVPadding: 10
        }

        property D.Palette background: D.Palette {
            normal: ("transparent")
            hovered: Qt.rgba(0, 0, 0, 0.1)
        }
    }

    // Button
    property QtObject button: QtObject {
        property int width: 140
        property int height: 36
        property int hPadding: control.radius
        property int vPadding: control.radius / 2.0
        property int iconSize: 24

        property D.Palette background1: D.Palette {
            normal {
                common: ("#f7f7f7")
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            hovered {
                common: ("#e1e1e1")
                crystal:  Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
            }
            pressed {
                common: ("#bcc4d0")
                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
            }
        }

        property D.Palette background2: D.Palette {
            normal {
                common: ("#f0f0f0")
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            hovered {
                common: ("#d2d2d2")
                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
            }
            pressed {
                common: ("#cdd6e0")
                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
            }
        }

        property D.Palette dropShadow: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.05)
            hovered: Qt.rgba(0, 0, 0, 0.1)
        }

        property D.Palette innerShadow1: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.05)
            pressed: ("transparent")
        }

        property D.Palette innerShadow2: D.Palette {
            normal: Qt.rgba(1, 1, 1, 0.2)
            hovered: Qt.rgba(1, 1, 1, 0.5)
            pressed: ("transparent")
        }

        property D.Palette insideBorder: D.Palette {
            normal {
                common: Qt.rgba(1, 1, 1, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.1)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            hovered {
                common: Qt.rgba(1, 1, 1, 0.2)
                crystal: Qt.rgba(0, 0, 0, 0.05)
            }
            pressed {
                common: Qt.rgba(1, 1, 1, 0.03)
                crystal: Qt.rgba(0, 0, 0, 0.03)
            }
        }

        property D.Palette outsideBorder: D.Palette {
            normal {
                common: Qt.rgba(0, 0, 0, 0.08)
                crystal: Qt.rgba(0, 0, 0, 0.08)
            }
            hovered: Qt.rgba(0, 0, 0, 0.2)
            pressed: ("transparent")
        }

        property D.Palette text: D.Palette {
            normal {
                common: Qt.rgba(0, 0, 0, 1)
                crystal: Qt.rgba(0, 0, 0, 1)
            }
            pressed {
                common: D.DTK.makeColor(D.Color.Highlight)
                crystal: D.DTK.makeColor(D.Color.Highlight)
            }
        }
    }

    property QtObject highlightedButton: QtObject {
        property D.Palette background1: D.Palette {
            normal {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+22).lightness(+14)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            normalDark {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+10).lightness(+10)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            hovered {
                common: D.DTK.makeColor(D.Color.Highlight).lightness(+10).saturation(+20).lightness(+10)
                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
            }
            hoveredDark {
                common: D.DTK.makeColor(D.Color.Highlight).lightness(+10).saturation(+30).lightness(+10)
            }
            pressed {
                common: D.DTK.makeColor(D.Color.Highlight).lightness(-10).saturation(+8).lightness(-8)
                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
            }
            pressedDark {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+10).lightness(+2)
            }
            disabled {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+22).lightness(+14)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
        }

        property D.Palette background2: D.Palette {
            normal {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+26).lightness(-10)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            normalDark {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+28).lightness(-2)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            hovered {
                common: D.DTK.makeColor(D.Color.Highlight).lightness(+10).saturation(+15).lightness(-8)
                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.2)
            }
            hoveredDark {
                common: D.DTK.makeColor(D.Color.Highlight).lightness(+10).saturation(+30).lightness(-7)
            }
            pressed {
                common: D.DTK.makeColor(D.Color.Highlight).lightness(-10).saturation(+15).lightness(-1)
                crystal: Qt.rgba(16.0 / 255, 16.0 / 255, 16.0 / 255, 0.15)
            }
            pressedDark {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+10).lightness(+2)
            }
            disabled {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+22).lightness(+14)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
        }

        property D.Palette dropShadow: D.Palette {
            normal {
                common: D.DTK.makeColor(D.Color.Highlight).saturation(+50).lightness(-10).opacity(-60)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            normalDark: D.DTK.makeColor(D.Color.Highlight).saturation(+15).lightness(+12).opacity(+20)
            pressed: D.DTK.makeColor(D.Color.Highlight).saturation(+50).lightness(-10).opacity(-80)
            disabled: D.DTK.makeColor(D.Color.Highlight).saturation(+50).lightness(-10).opacity(+30)
        }

        property D.Palette innerShadow1: D.Palette {
            normal {
                common: D.DTK.makeColor(D.Color.Highlight).lightness(-10)
                crystal: Qt.rgba(0, 0, 0, 0.1)
            }
            normalDark: D.DTK.makeColor(D.Color.Highlight).saturation(+25).lightness(-6)
            hovered: D.DTK.makeColor(D.Color.Highlight).lightness(+10).lightness(-10)
            pressed: D.DTK.makeColor(D.Color.Highlight).lightness(+10).lightness(-5)
            pressedDark: D.DTK.makeColor(D.Color.Highlight).saturation(+36).lightness(-10)
            disabled: D.DTK.makeColor(D.Color.Highlight).lightness(-10)
        }

        property D.Palette innerShadow2: D.Palette {
            normal: ("transparent")
        }

        property D.Palette text: D.Palette {
            normal {
                common: Qt.rgba(1, 1, 1, 1)
                crystal: D.DTK.makeColor(D.Color.Highlight)
            }
            normalDark: normal
        }

        property D.Palette border: D.Palette {
            normal {
                common: D.DTK.makeColor(D.Color.Highlight).opacity(+20)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            normalDark {
                common: Qt.rgba(1, 1, 1, 0.2)
                crystal: Qt.rgba(1, 1, 1, 0.1)
            }
            hovered: Qt.rgba(0, 0, 0, 0.03)
            hoveredDark: Qt.rgba(1, 1, 1, 0.2)
        }
    }

    property QtObject checkedButton: QtObject {
        property D.Palette background : D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight)
            hovered: D.DTK.makeColor(D.Color.Highlight).lightness(+10)
            pressed: D.DTK.makeColor(D.Color.Highlight).lightness(-10)
        }

        property D.Palette text : D.Palette {
            normal: D.DTK.makeColor(D.Color.HighlightedText)
            normalDark: D.DTK.makeColor(D.Color.HighlightedText)
            hovered: D.DTK.makeColor(D.Color.HighlightedText).lightness(+10)
            hoveredDark: D.DTK.makeColor(D.Color.HighlightedText).lightness(+10)
            pressed: D.DTK.makeColor(D.Color.HighlightedText).opacity(-20)
            pressedDark: D.DTK.makeColor(D.Color.HighlightedText).opacity(-20)
        }

        property D.Palette dropShadow : D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight).opacity(-60)
            pressed: D.DTK.makeColor(D.Color.Highlight).opacity(-80)
        }

        property D.Palette innerShadow : D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight).lightness(-10)
            hovered: D.DTK.makeColor(D.Color.Highlight)
            pressed: D.DTK.makeColor(D.Color.Highlight).lightness(-20)
        }
    }

    property QtObject windowButton: QtObject {
        property int width: 50
        property int height: 50
        property D.Palette background: D.Palette {
            normal: ("transparent")
            normalDark: ("transparent")
            hovered: Qt.rgba(0, 0, 0, 0.10)
            hoveredDark: Qt.rgba(1, 1, 1, 0.10)
            pressed: Qt.rgba(0, 0, 0, 0.15)
            pressedDark: Qt.rgba(1, 1, 1, 0.15)
        }
    }

    property QtObject warningButton: QtObject {
        property D.Palette text: D.Palette {
            normal: ("#ff5736")
        }
    }

    property QtObject switchButton: QtObject {
        property int indicatorWidth: 50
        property int indicatorHeight: 24
        property int handleWidth: 30
        property int handleHeight: 24
        property string iconName: "switch_button"

        property D.Palette background: D.Palette {
            normal: Qt.rgba(50 / 255, 50 / 255, 50 / 255, 0.2)
            normalDark: ("#1A1A1A")
        }

        property D.Palette handle: D.Palette {
            normal: ("#8c8c8c")
            normalDark: ("#444444")
        }
    }

    property QtObject floatingButton: QtObject {
        property int size: 24
    }

    property QtObject iconButton: QtObject {
        property int backgroundSize: 36
        property int iconSize: 16
        property int padding: 9
    }

    property QtObject toolButton: QtObject {
        property int width: 30
        property int height: 30
        property int iconSize: 16
        property int indicatorRightMargin: 6
    }

    property QtObject radioButton: QtObject {
        property int indicatorSize: 16
        property int iconSize: 16
        property int spacing: 8
        property int topPadding: 12
        property int bottomPadding: 12
    }

    // Box
    property QtObject checkBox: QtObject {
        property int indicatorWidth: 16
        property int indicatorHeight: 16
        property int padding: 2
        property int iconSize: 16
        property int focusRadius: 4
    }

    property QtObject buttonBox: QtObject {
        property int width: 60
        property int height: 30
        property int padding: 0
        property int spacing: 0
    }

    property QtObject comboBox: QtObject {
        property int width: 240
        property int height: 36
        property int padding: 8
        property int spacing: 10
        property int iconSize: 12
        property int maxVisibleItems: 16

        property QtObject edit: QtObject {
            property int indicatorSpacing: 7
            property int indicatorSize: 24

            property D.Palette separator: D.Palette {
                normal: Qt.rgba(0, 0, 0, 0.05)
            }
        }
    }

    // Edit
    property QtObject edit: QtObject {
        property int width: 180
        property int actionIconSize: 16
        property int textFieldHeight: 36
        property int textAreaHeight: 100

        property D.Palette background: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.08)
            normalDark: Qt.rgba(1, 1, 1, 0.1)
        }

        property D.Palette alertBackground: D.Palette {
            normal: Qt.rgba(0.95, 0.22, 0.20, 0.15)
        }

        property D.Palette placeholderText: D.Palette {
            normal: Qt.rgba(0.33, 0.33, 0.33, 0.4)
            normalDark: Qt.rgba(1, 1, 1, 0.3)
        }
    }

    property QtObject searchEdit: QtObject {
        property int iconSize: 16
        property int iconLeftMargin: 10
        property int iconRightMargin: 7
        property int animationDuration: 200
    }

    property QtObject ipEdit: QtObject {
        property int fieldWidth: 40
    }

    property QtObject passwordEdit: QtObject {
        property int echoActionSpacing: 8
    }

    property QtObject keySequenceEdit: QtObject {
        property int width: 200
        property int height: 36
        property int margin: 10


        property D.Palette background: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.05)
            normalDark: Qt.rgba(255, 255, 255, 0.05)
        }

        property D.Palette placeholderText: D.Palette {
            normal: Qt.rgba(85 / 255, 85 / 255, 85 / 255, 0.4)
            normalDark: Qt.rgba(192 / 255, 198 / 255, 212 / 255, 0.4)
        }

        property QtObject label: QtObject {
            property int leftRightPadding: 6
            property int topBottomPadding: 3

            property D.Palette background1: D.Palette {
                normal: ("#c3c3c3")
                normalDark: ("#3e3e3e")
            }

            property D.Palette background2: D.Palette {
                normal: ("#d8d8d8")
                normalDark: ("#525252")
            }

            property D.Palette shadowInner1: D.Palette {
                normal: Qt.rgba(0, 0, 0, 0.1)
                normalDark: Qt.rgba(0, 0, 0, 0.2)
            }

            property D.Palette shadowInner2: D.Palette {
                normalDark: Qt.rgba(1, 1, 1, 0.1)
            }

            property D.Palette shadowOuter: D.Palette {
                normal: Qt.rgba(0, 0, 0, 0.3)
            }

            property D.Palette text: D.Palette {
                normal: ("black")
            }
        }
    }

    property QtObject spinBox: QtObject {
        property int width : 300
        property int height: 36
        property int spacing: 10

        property QtObject indicator: QtObject {
            property int width: 24
            property int height: 14
            property int iconSize: 24
            property int focusIconSize: 10

            property D.Palette background: D.Palette {
                normal: Qt.rgba(0, 0, 0, 0.7)
                hovered: Qt.rgba(0, 0, 0, 0.6)
                pressed: Qt.rgba(0, 0, 0, 0.8)
            }
        }
    }

    property QtObject plusMinusSpinBox: QtObject {
        property int buttonIconSize: 16
    }

    // Window
    property QtObject dialogWindow: QtObject {
        property int width: 120
        property int height: 120
        property int contentHMargin: 10
        property int footerMargin: 10
        property int titleBarHeight: 50
        property int iconSize: 32
    }

    property QtObject aboutDialog: QtObject {
        property int width: 540
        property int height: 290
        property int leftAreaWidth: 220
        property int bottomPadding: 20
        property int productIconHeight: 128
    }

    property QtObject popup: QtObject {
        property int width: 80
        property int height: 180
        property int radius: 18
        property int padding: 10
    }

    property QtObject floatingMessage: QtObject {
        property int maximumWidth: 450
        property int minimumHeight: 40
        property int closeButtonSize: 24
    }

    property QtObject floatingPanel: QtObject {
        property int width: 180
        property int height: 40
        property int radius: 14
        property D.Palette background: D.Palette {
            normal: behindWindowBlur.lightColor
            normalDark: behindWindowBlur.darkColor
        }

        property D.Palette backgroundNoBlur: D.Palette {
            normal: behindWindowBlur.lightNoBlurColor
            normalDark: behindWindowBlur.darkNoBlurColor
        }

        property D.Palette dropShadow: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.2)
            normalDark: Qt.rgba(0, 0, 0, 0.2)
        }

        property D.Palette outsideBorder: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.05)
            normalDark: Qt.rgba(0, 0, 0, 0.5)
        }

        property D.Palette insideBorder: D.Palette {
            normal: Qt.rgba(1, 1, 1, 0.05)
            normalDark: Qt.rgba(1, 1, 1, 0.05)
        }
    }

    property QtObject toolTip: QtObject {
        property int verticalPadding: 4
        property int horizontalPadding: 5
        property int height: 24
    }

    property QtObject alertToolTip: QtObject {
        property int connectorWidth: 3
        property int connectorHeight: 12
        property int verticalPadding: 4
        property int horizontalPadding: 10

        property D.Palette text: D.Palette {
            normal: ("#e15736")
            normalDark: ("#e13669")
        }

        property D.Palette background: D.Palette {
            normal: Qt.rgba(247 / 255, 247 / 255, 247 / 255, 0.6)
            normalDark: Qt.rgba(59 / 255, 59 / 255, 59 / 255, 0.6)
        }

        property D.Palette connecterdropShadow: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.1)
        }

        property D.Palette connecterBackground: D.Palette {
            normal: Qt.rgba(1, 1, 1, 1)
            normalDark: Qt.rgba(0, 0, 0, 0.1)
        }
    }

    property QtObject menu: QtObject {
        property int padding: 0
        property int radius: D.DTK.platformTheme.windowRadius < 0 ? 18 : D.DTK.platformTheme.windowRadius
        property int margins: 10
        property int overlap: 1

        property QtObject item: QtObject {
            property int width: 180
            property int height: 34
            property size iconSize: Qt.size(14, 14)
            property int count: 0
            property int contentPadding: 36
            property int indicatorMargin: 10
        }

        property QtObject separator: QtObject {
            property int lineTopPadding : 6
            property int lineBottomPadding : 4
            property int lineHeight : 2
            property int topPadding: 11
            property int bottomPadding: 2

            property D.Palette lineColor: D.Palette {
                normal: Qt.rgba(0, 0, 0, 0.1)
            }
        }

        property D.Palette background: D.Palette {
            normal: behindWindowBlur.lightColor
            normalDark: behindWindowBlur.darkColor
        }

        property D.Palette backgroundNoBlur: D.Palette {
            normal: behindWindowBlur.lightNoBlurColor
            normalDark: behindWindowBlur.darkNoBlurColor
        }

        property D.Palette subMenuOpenedBackground: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.15)
        }

        property D.Palette itemText: D.Palette {
            normal: ("black")
            normalDark: Qt.rgba(1, 1, 1, 0.6)
        }

        property D.Palette separatorText: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.5)
            normalDark: Qt.rgba(1, 1, 1, 0.4)
        }
    }

    // Panel
    property QtObject highlightPanel: QtObject {
        property int width: 180
        property int height: 30

        property D.Palette background: D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight)
            normalDark: D.DTK.makeColor(D.Color.Highlight)
            hovered: D.DTK.makeColor(D.Color.Highlight).lightness(+10)
            hoveredDark: D.DTK.makeColor(D.Color.Highlight).lightness(+10)
        }
        property D.Palette dropShadow:  D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight).lightness(+20).opacity(-80)
            normalDark: D.DTK.makeColor(D.Color.Highlight).lightness(+20).opacity(-80)
        }
        property D.Palette innerShadow:  D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight).lightness(-20)
            normalDark: D.DTK.makeColor(D.Color.Highlight).lightness(-20)
        }
    }

    property QtObject behindWindowBlur: QtObject {
        property color lightColor: Qt.rgba(235 / 255.0, 235 / 255.0, 235 / 255.0, 0.6)
        property color lightNoBlurColor: Qt.rgba(235 / 255.0, 235 / 255.0, 235 / 255.0, 1.0)
        property color darkColor: "#55000000"
        property color darkNoBlurColor: Qt.rgba(35 / 255.0, 35 / 255.0, 35 / 255.0, 1.0)
    }

    property QtObject arrowRectangleBlur: QtObject {
        property int borderWidth: 1
        property int roundJoinRadius: 8
        property color outBorderColor: Qt.rgba(0, 0, 0, 0.06)
        property color darkOutBorderColor: Qt.rgba(0, 0, 0, 0.8)
        property color inBorderColor: Qt.rgba(1, 1, 1, 0.1)
        property color darkInBorderColor: Qt.rgba(1, 1, 1, 0.8)
        property color backgroundColor: behindWindowBlur.lightColor
        property color darkBackgroundColor: behindWindowBlur.darkColor
        property color backgroundNoBlurColor: behindWindowBlur.lightNoBlurColor
        property color darkBackgroundNoBlurColor: behindWindowBlur.darkNoBlurColor
        property color shadowColor: Qt.rgba(0, 0, 0, 0.2)
        property color darkShadowColor: Qt.rgba(0, 0, 0, 0.2)
    }

    property QtObject arrowListView: QtObject {
        property int width: 180
        property size stepButtonSize: Qt.size(16, 16)
        property size stepButtonIconSize: Qt.size(12, 12)
        property int maxVisibleItems: 16
        property int itemHeight: 30
        property string upButtonIconName: "arrow_ordinary_up"
        property string downButtonIconName: "arrow_ordinary_down"
    }

    property QtObject itemDelegate: QtObject {
        property int width: 204
        property int height: 40
        property color normalColor: Qt.rgba(0, 0, 0, 0.05)
        property color cascadeColor: Qt.rgba(0, 0, 0, 0.15)
        property int iconSize: 24
        property int checkIndicatorIconSize: 24
        property D.Palette checkBackgroundColor: D.Palette {
            hovered: Qt.rgba(0, 0, 0, 0.1)
            normal: Qt.rgba(0, 0, 0, 0.05)
        }
        property color checkedColor: Qt.rgba(0, 0, 0, 0.15)
    }

    property QtObject stackView: QtObject {
        property int animationDuration: 200
        property int animationEasingType: Easing.OutCubic
    }

    // Indicator
    property QtObject busyIndicator: QtObject {
        property int size: 16
        property int paddingFactor: 16
        property D.Palette fillColor: D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight)
        }
        property int animationDuration: 800
        property string spinnerSource: "qrc:/dtk/declarative/resources/spinner-blue.webp"
    }

    property QtObject buttonIndicator: QtObject {
        property int size: 16
        property int iconSize: 12
        property D.Palette background: D.Palette {
            normal: Qt.rgba(1, 1, 1, 0.2)
            normalDark: ("#ffffff")
        }
    }

    // Bar
    property QtObject scrollBar: QtObject {
        property int padding: 2
        property int width: 4
        property int activeWidth: 12
        property real hideOpacity: 0.0
        property int hidePauseDuration: 450
        property int hideDuration: 1500

        property D.Palette background: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.3)
            normalDark: Qt.rgba(1, 1, 1, 0.3)

            hovered: Qt.rgba(0, 0, 0, 0.4)
            hoveredDark: Qt.rgba(1, 1, 1, 0.6)

            pressed: Qt.rgba(0, 0, 0, 0.4)
            pressedDark: Qt.rgba(1, 1, 1, 0.3)
        }

        property D.Palette outsideBorder: D.Palette {
            normal: Qt.rgba(1, 1, 1, 0.1)
            normalDark: Qt.rgba(0, 0, 0, 0.1)
        }

        property D.Palette insideBorder: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.05)
            normalDark: Qt.rgba(1, 1, 1, 0.05)
        }
    }

    property QtObject progressBar: QtObject {
        property int width: 300
        property int height: 36
        property int indeterminateProgressBarWidth: 90
        property int indeterminateProgressBarAnimationDuration: 2000

        property D.Palette shadowColor:  D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight).opacity(-60)
        }

        property D.Palette handleGradientColor:  D.Palette {
            normal: D.DTK.makeColor(D.Color.Highlight).hue(-10).saturation(+40).lightness(+20)
        }
    }

    property QtObject embeddedProgressBar: QtObject {
        property int width: 48
        property int height: 6
        property int contentHeight: 4
        property int backgroundRadius: 3
        property int contentRadius: 2

        property D.Palette background: D.Palette {
            normal: Qt.rgba(0, 0, 0, 0.7)
            normalDark: Qt.rgba(0, 0, 0, 0.2)
        }

        property D.Palette progressBackground: D.Palette {
            normal: ("white")
            normalDark: Qt.rgba(1, 1, 1, 0.7)
        }
    }

    property QtObject waterProgressBar: QtObject {
        property int width: 98
        property int height: 98
        property string waterFrontImagePath: "qrc:/dtk/declarative/resources/water_front.png"
        property string waterBackImagePath: "qrc:/dtk/declarative/resources/water_back.png"

        property D.Palette background1: D.Palette {
            normal: ("#1f08ff")
        }

        property D.Palette background2: D.Palette {
            normal: ("#50fff7")
        }

        property D.Palette dropShadow: D.Palette {
            normal: Qt.rgba(42 / 255, 154 / 255, 248 / 255, 0.3)
        }

        property D.Palette popBackground: D.Palette {
            normal: Qt.rgba(255, 255, 255, 0.3)
        }

        property D.Palette textColor: D.Palette {
            normal: ("white")
        }
    }

    property QtObject titleBar: QtObject {
        property int height: 50
        property int iconSize: 32
        property int leftMargin: 10
    }

    // Other
    property QtObject slider: QtObject {
        property int width: 120
        property int height: 60
        property int highlightMargin: -4

        property QtObject handle: QtObject {
            property int width: 20
            property int height: 24
        }

        property QtObject groove: QtObject {
            property int width: 100
            property int height: 4

            property D.Palette background: D.Palette {
                normal: Qt.rgba(0, 0, 0, 0.2)
                normalDark: Qt.rgba(1, 1, 1, 0.2)
            }
        }

        property QtObject tick: QtObject {
            property int width: 1
            property int height: 12
            property int radius: 8
            property int textMargin: 2
            property int hPadding: 10
            property int vPadding: 2

            property D.Palette background: D.Palette {
                normal: Qt.rgba(0, 0, 0, 0.4)
                normalDark: Qt.rgba(1, 1, 1, 0.4)
            }
        }
    }

    // Dial
    property QtObject dial: QtObject {
        property int size: 100
    }

    // PageIndicator
    property QtObject pageIndicator: QtObject {
        property int width: 8
        property int height: 8
    }
}
