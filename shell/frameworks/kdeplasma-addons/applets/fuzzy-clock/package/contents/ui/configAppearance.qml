/*
 * SPDX-FileCopyrightText: 2013 Bhushan Shah <bhush94@gmail.com>
 * SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.0
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.0
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    property alias cfg_boldText: boldCheckBox.checked
    property alias cfg_italicText: italicCheckBox.checked

    property alias cfg_fuzzyness: fuzzyness.value

    Kirigami.FormLayout {
        QQC2.CheckBox {
            id: boldCheckBox
            Kirigami.FormData.label: i18nc("@title:group", "Font:")
            text: i18nc("@option:check", "Bold text")
        }

        QQC2.CheckBox {
            id: italicCheckBox
            text: i18nc("@option:check", "Italic text")
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.Slider {
            id: fuzzyness
            Kirigami.FormData.label: i18nc("@title:group", "Fuzzyness:")
            from: 1
            to: 5
            stepSize: 1
        }

        RowLayout {
        Layout.fillWidth: true

            QQC2.Label {
                text: i18nc("@item:inrange", "Accurate")
                textFormat: Text.PlainText
            }

            Item {
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: i18nc("@item:inrange", "Fuzzy")
                textFormat: Text.PlainText
            }
        }
    }
}
