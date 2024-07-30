/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.private.kcm.kscreen 1.0 as KScreen

QQC2.ToolButton {
    id: root

    property int value
    property /*KScreen::Output::Rotation*/int outputRotation
    property string tooltip

    Layout.fillWidth: true
    height: childrenRect.height

    checkable: true
    checked: element.rotation === outputRotation

    QQC2.ToolTip {
        text: tooltip
    }

    contentItem: Kirigami.Icon {
        source: "tv"
        rotation: root.value
    }

    onClicked: {
        if (element.rotation === outputRotation) {
            return;
        }

        element.rotation = outputRotation;
        screen.resetTotalSize();
    }

    implicitWidth: contentItem.implicitWidth + 2 * Kirigami.Units.smallSpacing
    implicitHeight: contentItem.implicitHeight + 2 * Kirigami.Units.smallSpacing

    Component.onCompleted: {
        switch (value) {
        case 90:
            outputRotation = KScreen.Output.Left;
            tooltip = i18n("90° Clockwise");
            break;
        case 180:
            outputRotation = KScreen.Output.Inverted;
            tooltip = i18n("Upside Down");
            break;
        case 270:
            outputRotation = KScreen.Output.Right;
            tooltip = i18n("90° Counterclockwise")
            break;
        case 0:
        default:
            outputRotation = KScreen.Output.None;
            tooltip = i18n("No Rotation");
        }
    }
}
