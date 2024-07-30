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
    heading: i18nc("@info:window The name of a KWin effect", "Overview")
    description: xi18nc("@info:usagetip", "Overview is a full-screen overlay that shows all of your open windows, letting you easily access any of them. It also shows your current Virtual Desktops, allowing you to add more, remove some, and switch between them. Finally, it offers a KRunner-powered search field that can also filter through open windows.<nl/><nl/>You can access Overview using the <shortcut>Meta+W</shortcut> keyboard shortcut.")
}
