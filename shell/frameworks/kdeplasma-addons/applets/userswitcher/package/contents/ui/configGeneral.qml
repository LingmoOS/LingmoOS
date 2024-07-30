/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QtControls

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property bool cfg_showFace
    property bool cfg_showName
    property bool cfg_showFullName
    property alias cfg_showTechnicalInfo: showTechnicalInfoCheck.checked

    Kirigami.FormLayout {
        QtControls.ButtonGroup {
            id: nameGroup
        }

        QtControls.RadioButton {
            id: showFullNameRadio

            Kirigami.FormData.label: i18nc("@title:label", "Username style:")

            QtControls.ButtonGroup.group: nameGroup
            text: i18nc("@option:radio", "Full name (if available)")
            checked: cfg_showFullName
            onClicked: if (checked) cfg_showFullName = true;
        }

        QtControls.RadioButton {
            QtControls.ButtonGroup.group: nameGroup
            text: i18nc("@option:radio", "Login username")
            checked: !cfg_showFullName
            onClicked: if (checked) cfg_showFullName = false;
        }


        Item {
            Kirigami.FormData.isSection: true
        }


        QtControls.ButtonGroup {
            id: layoutGroup
        }

        QtControls.RadioButton {
            id: showOnlyNameRadio

            Kirigami.FormData.label: i18nc("@title:label", "Show:")

            QtControls.ButtonGroup.group: layoutGroup
            text: i18nc("@option:radio", "Name")
            checked: cfg_showName && !cfg_showFace
            onClicked: {
                if (checked) {
                    cfg_showName = true;
                    cfg_showFace = false;
                }
            }
        }

        QtControls.RadioButton {
            id: showOnlyFaceRadio

            QtControls.ButtonGroup.group: layoutGroup
            text: i18nc("@option:radio", "User picture")
            checked: !cfg_showName && cfg_showFace
            onClicked: {
                if (checked) {
                    cfg_showName = false;
                    cfg_showFace = true;
                }
            }
        }

        QtControls.RadioButton {
            id: showBothRadio

            QtControls.ButtonGroup.group: layoutGroup
            text: i18nc("@option:radio", "Name and user picture")
            checked: cfg_showName && cfg_showFace
            onClicked: {
                if (checked) {
                    cfg_showName = true;
                    cfg_showFace = true;
                }
            }
        }


        Item {
            Kirigami.FormData.isSection: true
        }


        QtControls.CheckBox {
            id: showTechnicalInfoCheck

            Kirigami.FormData.label: i18nc("@title:label", "Advanced:")

            text: i18nc("@option:check", "Show technical session information")
        }
    }
}
