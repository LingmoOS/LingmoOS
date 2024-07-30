/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Templates as T

import org.kde.ksvg as KSvg
//NOTE: importing PlasmaCore is necessary in order to make KSvg load the current Plasma Theme
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import "private" as Private

KSvg.FrameSvgItem {
    id: root

    required property T.AbstractButton control

    imagePath: "widgets/button"
    prefix: "normal"
    implicitWidth: Kirigami.Units.iconSizes.small
    implicitHeight: Kirigami.Units.iconSizes.small
    opacity: control.enabled ? 1 : 0.6

    Private.ButtonShadow {
        anchors.fill: parent
        showShadow: !control.down
    }

    KSvg.SvgItem {
        anchors.fill: parent
        svg: KSvg.Svg {
            id: checkmarkSvg
            imagePath: "widgets/checkmarks"
        }
        elementId: "checkbox"
        opacity: {
            if (control instanceof T.CheckBox) {
                switch (control.checkState) {
                case Qt.Checked:
                    return 1;
                case Qt.PartiallyChecked:
                    return 0.5;
                default:
                    return 0;
                }
            } else {
                return control.checked ? 1 : 0;
            }
        }
        Behavior on opacity {
            enabled: Kirigami.Units.longDuration > 0
            NumberAnimation {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    Private.ButtonFocus {
        anchors.fill: parent
        showFocus: control.visualFocus && !control.down
    }

    Private.ButtonHover {
        anchors.fill: parent
        showHover: control.hovered
    }
}
