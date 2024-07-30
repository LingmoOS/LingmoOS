// SPDX-FileCopyrightText: 2013-2017 Jan Grulich <jgrulich@redhat.com>
// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL

import QtQuick 2.15
import org.kde.lingmoui 2.10 as LingmoUI
import org.kde.lingmo.networkmanagement as LingmoNM

LingmoUI.PasswordField {
    property int securityType
    placeholderText: i18n("Password…")
    validator: RegularExpressionValidator {
                    regularExpression: if (securityType == LingmoNM.Enums.StaticWep) {
                                /^(?:.{5}|[0-9a-fA-F]{10}|.{13}|[0-9a-fA-F]{26}){1}$/
                            } else {
                                /^(?:.{8,64}){1}$/
                            }
                    }
}

