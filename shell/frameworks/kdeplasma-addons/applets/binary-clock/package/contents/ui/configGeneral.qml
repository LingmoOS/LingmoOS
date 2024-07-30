/*
 *  SP9X-FileCopyrightText: 2014 Joseph Wenninger <jowenn@kde.org>
 *  SPDX-FileCopyrightText: 2018 Piotr Kąkol <piotrkakol@protonmail.com>
 *
 *  Based on analog-clock configGeneral.qml:
 *  SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as QtControls
import QtQuick.Layouts 1.1
import org.kde.kquickcontrols 2.0 as KQuickControls
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_showOffLeds: showOffLedsCheckBox.checked
    property alias cfg_showSeconds: showSecondsCheckBox.checked
    property alias cfg_useCustomColorForActive: useCustomColorForActiveCheckBox.checked
    property alias cfg_customColorForActive: customColorForActive.color
    property alias cfg_useCustomColorForInactive: useCustomColorForInactiveCheckBox.checked
    property alias cfg_customColorForInactive: customColorForInactive.color

    Kirigami.FormLayout {
        QtControls.CheckBox {
            id: showOffLedsCheckBox
            text: i18nc("@option:check", "Inactive lights")
        }

        QtControls.CheckBox {
            id: showSecondsCheckBox
            text: i18nc("@option:check", "Seconds")
        }

        Item {
            Kirigami.FormData.isSection:true
        }

        RowLayout {
            Kirigami.FormData.label:i18n("Use custom color for:")

            QtControls.CheckBox {
                id: useCustomColorForActiveCheckBox
                text: i18nc("@option:check", "Active lights")
            }

            KQuickControls.ColorButton {
                id: customColorForActive
                enabled: useCustomColorForActiveCheckBox.checked
            }
        }

        RowLayout {

            QtControls.CheckBox {
                id: useCustomColorForInactiveCheckBox
                text: i18nc("@option:check", "Inactive lights")
            }

            KQuickControls.ColorButton {
                id:customColorForInactive
                enabled: useCustomColorForInactiveCheckBox.checked
            }
        }
    }
}
