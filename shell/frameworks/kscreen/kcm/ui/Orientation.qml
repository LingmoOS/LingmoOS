/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami

import org.kde.private.kcm.kscreen 1.0 as KScreen

ColumnLayout {
    Kirigami.FormData.label: i18n("Orientation:")
    Kirigami.FormData.labelAlignment: autoRotateColumn.visible ? Qt.AlignTop : Qt.AlignVCenter
    Kirigami.FormData.buddyFor: autoRotateColumn.visible ? autoRotateColumn : orientation
    spacing: Kirigami.Units.smallSpacing

    ColumnLayout {
        id: autoRotateColumn

        enabled: element.internal
        visible: kcm.autoRotationSupported && kcm.orientationSensorAvailable

        ColumnLayout {
            QQC2.RadioButton {
                id: autoRotateRadio
                text: i18n("Automatic")
                checked: autoRotateColumn.enabled && element.autoRotate != KScreen.Output.AutoRotatePolicy.Never
                onToggled: {
                    if (element.autoRotate == KScreen.Output.AutoRotatePolicy.Never) {
                        element.autoRotate = KScreen.Output.AutoRotatePolicy.InTabletMode
                    } else {
                        element.autoRotate = KScreen.Output.AutoRotatePolicy.Never
                    }
                }
            }

            QQC2.CheckBox {
                id: autoRotateOnlyInTabletMode
                Layout.leftMargin: Kirigami.Units.gridUnit

                text: i18n("Only when in tablet mode")
                enabled: autoRotateRadio.checked
                checked: enabled && element.autoRotate == KScreen.Output.AutoRotatePolicy.InTabletMode
                visible: kcm.tabletModeAvailable
                onToggled: {
                    if (element.autoRotate == KScreen.Output.AutoRotatePolicy.Always) {
                        element.autoRotate = KScreen.Output.AutoRotatePolicy.InTabletMode
                    } else {
                        element.autoRotate = KScreen.Output.AutoRotatePolicy.Always
                    }
                }
            }
        }

        QQC2.RadioButton {
            id: manualRotateRadio
            text: i18n("Manual")
            checked: element.autoRotate == KScreen.Output.AutoRotatePolicy.Never || !autoRotateColumn.enabled
            onToggled: element.autoRotate = KScreen.Output.AutoRotatePolicy.Never
        }
    }

    RowLayout {
       id: orientation
       enabled: !element.autoRotate || !autoRotateColumn.enabled || !autoRotateColumn.visible

       QQC2.ButtonGroup {
           buttons: orientation.children
       }

       RotationButton {
           value: 0
       }
       RotationButton {
           value: 90
       }
       RotationButton {
           value: 180
       }
       RotationButton {
           value: 270
       }
    }
}
