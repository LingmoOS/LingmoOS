/* SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Templates as T
import org.kde.lingmoui as LingmoUI
import org.kde.ocean

import "." as Impl

IconLabelLayout {
    id: root
    // NOTE: Remember to use root.mirrored, not control.mirrored in this file
    // Controls can change the mirrored property of this component and those
    // changes will be ignored if you use control.mirrored directly.
    property T.AbstractButton control: root.parent
    property bool reserveSpaceForIndicator: false
    property bool reserveSpaceForIcon: false
    property bool reserveSpaceForArrow: false
    property bool oppositeSideIndicator: {
        if (!control.indicator) { return false; }
        let indicatorCenter = control.indicator.x + control.indicator.width/2
        let controlCenter = control.width/2
        return root.mirrored ? indicatorCenter <= controlCenter : indicatorCenter > controlCenter
    }

    icon {
        name: control.icon.name
        source: control.icon.source
        width: control.icon.width
        height: control.icon.height
        color: control.icon.color
        cache: control.icon.cache
    }
    text: control.text
    font: control.font
    color: LingmoUI.Theme.textColor

    property real padding: 0
    property real horizontalPadding: padding
    property real verticalPadding: padding
    leftPadding: {
        let lpad = horizontalPadding
        if (!oppositeSideIndicator) {
            if (control.indicator && control.indicator.visible && control.indicator.width > 0) {
                lpad += control.indicator.width + root.spacing
            } else if (reserveSpaceForIndicator) {
                lpad += Units.inlineControlHeight + root.spacing
            }
        }
        if (reserveSpaceForIcon && !hasIcon && control.icon.width > 0) {
            lpad += control.icon.width + root.spacing
        }
        return lpad
    }
    rightPadding: {
        let rpad = horizontalPadding
        if (oppositeSideIndicator) {
            if (control.indicator && control.indicator.visible && control.indicator.width > 0) {
                rpad += control.indicator.width + root.spacing
            } else if (reserveSpaceForIndicator) {
                rpad += Units.inlineControlHeight + root.spacing
            }
        }
        if (control.arrow && (control.arrow.visible || reserveSpaceForArrow) && control.arrow.width > 0) {
            rpad += control.arrow.width + root.spacing
        }
        return rpad
    }
    topPadding: verticalPadding
    bottomPadding: verticalPadding
    spacing: control.spacing

    mirrored: control.mirrored
    alignment: Qt.AlignCenter
    display: control.display

    iconComponent: Component {
        LingmoUI.Icon {
            // This is set in IconLabelLayout
            property bool firstLayoutCompleted: false
            visible: valid
            Behavior on opacity {
                enabled: firstLayoutCompleted
                OpacityAnimator {
                    duration: LingmoUI.Units.shortDuration
                }
            }
        }
    }

    labelComponent: Component {
        Text {
            // This is set in IconLabelLayout
            property bool firstLayoutCompleted: false
            visible: text.length > 0
            linkColor: LingmoUI.Theme.linkColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            Behavior on x {
                enabled: firstLayoutCompleted
                XAnimator {
                    duration: LingmoUI.Units.shortDuration
                }
            }
            Behavior on y {
                enabled: firstLayoutCompleted
                YAnimator {
                    duration: LingmoUI.Units.shortDuration
                }
            }
            Behavior on opacity {
                enabled: firstLayoutCompleted
                OpacityAnimator {
                    duration: LingmoUI.Units.shortDuration
                }
            }
        }
    }
}
