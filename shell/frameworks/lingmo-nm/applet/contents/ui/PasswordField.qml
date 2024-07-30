/*
    SPDX-FileCopyrightText: 2013-2017 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

import QtQuick
import org.kde.lingmo.networkmanagement as LingmoNM
import org.kde.lingmo.extras as LingmoExtras

LingmoExtras.PasswordField {
    property int/*LingmoNM.Enums.SecurityType*/ securityType

    validator: RegularExpressionValidator {
        regularExpression: (securityType === LingmoNM.Enums.StaticWep)
            ? /^(?:.{5}|[0-9a-fA-F]{10}|.{13}|[0-9a-fA-F]{26})$/
            : /^(?:.{8,64})$/
    }
}
