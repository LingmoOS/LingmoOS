/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.lingmoui as LingmoUI

LingmoUI.ScrollablePage {
    title: qsTr("General")

    QQC2.CheckBox {
        LingmoUI.FormData.label: i18n("Something")
        text: i18n("Do something")
    }
}
