// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

import QtQuick 2.15
import QtQuick.Controls 2.9 as QQC2
import org.kcm.firewall 1.0

QQC2.TextField {
    id: ipField

    property bool ipv6: false

    placeholderText: i18n("Any IP address")

    validator: IPValidator {
        ipVersion: ipField.ipv6 ? IPValidator.IPVersion.IPv6 : IPValidator.IPVersion.IPv4
    }
}
