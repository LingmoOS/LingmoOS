/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
*/

#pragma once

#include <QGuiApplication>
#include <QPair>
#include <QPoint>

#include <variant>

enum class BarrierFailureReason {
    Diagonal,
    NotOnEdge,
    BetweenScreensOrDoesNotFill,
};

std::variant<BarrierFailureReason, QPair<QPoint, QPoint>> checkAndMakeBarrier(int x1, int y1, int x2, int y2, const QList<QRect> &screenGeometries);
