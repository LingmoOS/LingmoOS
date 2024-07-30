/*
 *   SPDX-FileCopyrightText: 2019 Marco Martin <mart@kde.org>
 *   SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 *   SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
 *   SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.ksysguard.faces as Faces


Faces.SensorFace {
    id: root
    contentItem: Kirigami.Icon {
        source: "view-process-system"
    }
}
