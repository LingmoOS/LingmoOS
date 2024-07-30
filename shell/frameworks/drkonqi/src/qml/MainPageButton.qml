// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import org.kde.kirigami 2.19 as Kirigami

import org.kde.drkonqi 1.0

QQC2.Button {
    display: T.AbstractButton.TextUnderIcon
    icon.width: Kirigami.Units.iconSizes.huge
    icon.height: icon.width
    visible: action.visible
}
