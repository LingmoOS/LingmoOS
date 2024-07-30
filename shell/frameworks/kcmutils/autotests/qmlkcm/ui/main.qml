/**
 * SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@kde.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls

import org.kde.lingmoui 2.7 as LingmoUI
import org.kde.kcmutils

SimpleKCM {
    ConfigModule.buttons: ConfigModule.NoAdditionalButton // Just to show that setting the buttons works
    Controls.Label {
        text: i18n("Configure Time")
    }
}
