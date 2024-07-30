/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.plasma.welcome

GenericPage {
    heading: i18nc("@info:window", "Plasma Vaults")
    description: xi18nc("@info:usagetip", "Plasma Vaults allows you to create encrypted folders, called <interface>Vaults.</interface> Inside each Vault, you can securely store your passwords, files, pictures, and documents, safe from prying eyes. Vaults can live inside folders that are synced to cloud storage services too, providing extra privacy for that content.<nl/><nl/>To get started, click the arrow on the <interface>System Tray</interface> to show hidden items, and then click the <interface>Vaults</interface> icon.")
}
