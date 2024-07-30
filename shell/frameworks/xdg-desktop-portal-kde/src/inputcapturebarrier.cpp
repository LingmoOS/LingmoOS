/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
*/

#include "inputcapturebarrier.h"

#include <QScreen>

std::variant<BarrierFailureReason, QPair<QPoint, QPoint>> checkAndMakeBarrier(int x1, int y1, int x2, int y2, const QList<QRect> &screenGeometries)
{
    // This function checks and  allows barriers that are
    // - fully on a  edge of a screen
    // - not next to any other screen

    if (x1 != x2 && y1 != y2) {
        return BarrierFailureReason::Diagonal;
    }

    bool foundScreen = false;

    bool transpose = false;
    if (x1 != x2) {
        std::swap(x1, y1);
        std::swap(x2, y2);
        transpose = true;
    }
    if (y1 > y2) {
        std::swap(y1, y2);
    }
    bool onRightEdge = false;
    for (auto geometry : screenGeometries) {
        if (transpose) {
            geometry = geometry.transposed();
            geometry.moveTo(geometry.y(), geometry.x());
        }

        if (y1 > geometry.bottom() || geometry.y() > y2) {
            continue;
        }
        if (x1 == geometry.x() || x1 == geometry.x() + geometry.width()) {
            if (y1 == geometry.y() && y2 == geometry.bottom() && !foundScreen) {
                foundScreen = true;
                onRightEdge = x1 == geometry.x() + geometry.width();
            } else {
                // the edge one or doesnt fill the edge of this screen or it fills the edge of some other screen
                // that is next to this screen; either way we dont allow it
                return BarrierFailureReason::BetweenScreensOrDoesNotFill;
            }
        }
    }

    if (!foundScreen) {
        return BarrierFailureReason::NotOnEdge;
    }
    if (onRightEdge) {
        // Barriers on right/top edge will have a coordinate of just past the screen (on 1920 pixel wide screen at 0x0 1920)
        // We send coordinates on the screen to KWin which is consistent with the other case which sends the coordinate
        // of the first row/column of pixels
        --x1;
        --x2;
    }
    if (transpose) {
        std::swap(x1, y1);
        std::swap(x2, y2);
    }
    return QPair<QPoint, QPoint>{{x1, y1}, {x2, y2}};
}
