/*
SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.15

Rectangle {
    color: "white"

    RowLayout {
        Button {
            enabled: powerManagement.canSuspend
            text: "Suspend"
            onClicked: powerManagement.suspend()
        }
        Button {
            enabled: powerManagement.canHibernate
            text: "Hibernate"
            onClicked: powerManagement.hibernate()
        }
    }
}
