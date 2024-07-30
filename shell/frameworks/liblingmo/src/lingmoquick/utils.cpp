/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "utils.h"

Qt::Edge LingmoQuickPrivate::oppositeEdge(Qt::Edge edge)
{
    switch (edge) {
    case Qt::TopEdge:
        return Qt::BottomEdge;
    case Qt::BottomEdge:
        return Qt::TopEdge;
    case Qt::LeftEdge:
        return Qt::RightEdge;
    case Qt::RightEdge:
        return Qt::LeftEdge;
    }
    Q_UNREACHABLE();
}
