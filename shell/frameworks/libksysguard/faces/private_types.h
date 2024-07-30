/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQmlEngine>
#include <QTransposeProxyModel>

struct QTransposeProxyModelForeign {
    Q_GADGET
    QML_NAMED_ELEMENT(QTransposeProxyModel)
    QML_FOREIGN(QTransposeProxyModel)
};
