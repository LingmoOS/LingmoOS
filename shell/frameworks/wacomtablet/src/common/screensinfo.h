/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "screenrotation.h"

#include <QList>
#include <QMap>
#include <QRectF>

namespace Wacom
{

/**
 * @brief Various display info helper functions
 */
namespace ScreensInfo
{
/**
 * @return All screens united as one rectangle.
 */
const QRect getUnifiedDisplayGeometry();

const QMap<QString, QRect> getScreenGeometries();

/**
 * Gets the current rotation of the screen. This is based on the monitor
 * rotation and not the rotation of the canvas. If the monitor is e.g.
 * rotated to the right, then the canvas is rotated to the left.
 *
 * @return The current rotation as seen by the monitor.
 */
const ScreenRotation getScreenRotation(QString output = QString());

const QString getPrimaryScreenName();
const QString getNextScreenName(QString output);
}; // CLASS
} // NAMESPACE
