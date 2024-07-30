/*
 *  SPDX-FileCopyrightText: 2021 Felipe Kinoshita <kinofhek@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick

import org.kde.plasma.welcome

KCMPage {
    id: kcm_feedback

    heading: i18nc("@title: window", "Share Anonymous Usage Information")
    description: i18nc("@info:usagetip", "Our developers will use this anonymous data to improve KDE software. You can choose how much to share in System Settings, and here too.")
    path: "kcm_feedback"
}
