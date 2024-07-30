/*
    SPDX-FileCopyrightText: 2020 Niccolò Venerandi <niccolo@venerandi.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T

import org.kde.lingmo.core as LingmoCore
import org.kde.ksvg as KSvg
import org.kde.lingmo.plasmoid
import org.kde.lingmoui as LingmoUI

/**
 * Item to be used as a header or footer in plasmoids
 *
 * @inherit QtQuick.Templates.ToolBar
 */
T.ToolBar {
    id: control

    Layout.fillWidth: true
    bottomPadding: position === T.ToolBar.Footer ? 0 : -backgroundMetrics.getMargin("bottom")
    topPadding: position === T.ToolBar.Footer ? -backgroundMetrics.getMargin("top") : 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    leftInset: backgroundMetrics.getMargin("left")
    rightInset: backgroundMetrics.getMargin("right")
    topInset: position === T.ToolBar.Footer ? 0 : backgroundMetrics.getMargin("top")
    bottomInset: position === T.ToolBar.Footer ? backgroundMetrics.getMargin("bottom") : 0

    LingmoUI.Theme.colorSet: position === T.ToolBar.Header ? LingmoUI.Theme.Header : LingmoUI.Theme.Window
    LingmoUI.Theme.inherit: false

    property /*Qt.Edges*/ int enabledBorders: {
        const w = Window.window;
        const popup = w as LingmoCore.PopupLingmoWindow;
        if (!popup) {
            return Qt.LeftEdge | Qt.TopEdge | Qt.RightEdge | Qt.BottomEdge;
        }

        const windowBorders = popup.borders;
        let borders = Qt.TopEdge | Qt.BottomEdge;

        if (windowBorders & Qt.LeftEdge && background.LingmoUI.ScenePosition.x <= 0) {
            borders |= Qt.LeftEdge;
        }
        if (windowBorders & Qt.RightEdge && background.LingmoUI.ScenePosition.x + background.width >= w.width) {
            borders |= Qt.RightEdge;
        }

        return borders;
    }
    background: KSvg.FrameSvgItem {
        id: headingSvg
        // This graphics has to back with the dialog background, so it can be used if:
        // * both this and the dialog background are from the current theme
        // * both this and the dialog background are from fallback
        visible: fromCurrentImageSet === backgroundSvg.fromCurrentImageSet
        imagePath: "widgets/plasmoidheading"
        prefix: control.position === T.ToolBar.Header ? "header" : "footer"
        KSvg.Svg {
            id: backgroundSvg
            imagePath: "dialogs/background"
        }

        enabledBorders: {
            let borders = KSvg.FrameSvg.NoBorder;
            if (control.enabledBorders & Qt.LeftEdge) {
                borders |= KSvg.FrameSvg.LeftBorder;
            }
            if (control.enabledBorders & Qt.RightEdge) {
                borders |= KSvg.FrameSvg.RightBorder;
            }
            if (control.enabledBorders & Qt.TopEdge) {
                borders |= KSvg.FrameSvg.TopBorder;
            }
            if (control.enabledBorders & Qt.BottomEdge) {
                borders |= KSvg.FrameSvg.BottomBorder;
            }
            return borders;
        }

        BackgroundMetrics {
            id: backgroundMetrics

            function getMargin(margin: string): real {
                const w = Window.window;

                // TODO: This shouldn't be duck-typed
                if (w && w.hasOwnProperty("leftPadding")
                      && w.hasOwnProperty("topPadding")
                      && w.hasOwnProperty("rightPadding")
                      && w.hasOwnProperty("bottomPadding")) {
                    switch (margin) {
                    case "left":
                        return -w.leftPadding;
                    case "top":
                        return -w.topPadding;
                    case "right":
                        return -w.rightPadding;
                    case "bottom":
                    default:
                        return -w.bottomPadding;
                    }
                } else if (!hasInset) {
                    return -headingSvg.fixedMargins[margin];
                } else {
                    return -fixedMargins[margin] + inset[margin];
                }
            }
        }
    }
}
