/*
 *  SPDX-FileCopyrightText: 2018 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls

import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    signal configurationChanged()

    property var cfg_key: []

    function toggle(name, checked) {
        const index = cfg_key.indexOf(name);

        if (checked) {
            if (index < 0) {
                cfg_key.push(name);
            }
        } else if (index >= 0) {
            cfg_key.splice(index, 1);
        }

        configurationChanged();
    }

    Kirigami.FormLayout {

        Controls.CheckBox {
            Kirigami.FormData.label: i18nc("@label show keyboard indicator when Caps Lock or Num Lock is activated", "Show when activated:")
            readonly property string name: "Caps Lock"
            checked: cfg_key.indexOf(name) >= 0
            text: i18nc("@option:check", "Caps Lock")
            onToggled: root.toggle(name, checked)
        }

        Controls.CheckBox {
            readonly property string name: "Num Lock"
            checked: cfg_key.indexOf(name) >= 0
            text: i18nc("@option:check", "Num Lock")
            onToggled: root.toggle(name, checked)
        }
    }
}
