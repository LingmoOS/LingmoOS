/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

.import org.kde.kirigami 2.0 as Kirigami
.import org.kde.bolt 0.1 as Bolt

function deviceStatus(device, withStored)
{
    var status = device.status;
    var str = "";
    var color = Kirigami.Theme.textColor;
    if (status == Bolt.Bolt.Status.Disconnected) {
        str = i18n("Disconnected");
    } else if (status == Bolt.Bolt.Status.Connecting) {
        str = i18n("Connecting");
    } else if (status == Bolt.Bolt.Status.Connected) {
        str = i18n("Connected");
        color = Kirigami.Theme.neutralTextColor;
    } else if (status == Bolt.Bolt.Status.AuthError) {
        str = i18n("Authorization Error");
    } else if (status == Bolt.Bolt.Status.Authorizing) {
        str = i18n("Authorizing");
    } else if (status == Bolt.Bolt.Status.Authorized) {
        color = Kirigami.Theme.positiveTextColor;
        if (device.authFlags & Bolt.Bolt.Auth.NoPCIE) {
            str = i18n("Reduced Functionality");
        } else {
            str = i18n("Connected & Authorized");
        }
    }
    if (withStored && device.stored) {
        if (str != "") {
            str += ", ";
        }
        str += i18n("Trusted");
    }

    return { text: str, color: color };
}
