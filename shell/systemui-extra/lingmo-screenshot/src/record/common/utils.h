/*
 * Copyright (c) 2012-2020 Maarten Baert <maarten-baert@hotmail.com>

 * This file is part of Lingmo-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef UTILS_H
#define UTILS_H

#include "Global.h"
#include <QApplication>

std::vector<QRect> GetScreenGeometries();

QRect CombineScreenGeometries(const std::vector<QRect>& screen_geometries);

// This does some sanity checking on the rubber band rectangle before creating it.
// Rubber bands with width or height zero or extremely large appear to cause problems.
QRect ValidateRubberBandRectangle(QRect rect);

QRect MapToLogicalCoordinates(const QRect& rect);

QString GetNewSegmentFile(const QString& file, bool add_timestamp);

QString ReadableTime(int64_t time_micro);

QString ReadableWidthHeight(unsigned int width, unsigned int height);

QString ReadableSizeIEC(uint64_t size, const QString& suffix);

QString ReadableSizeSI(uint64_t size, const QString& suffix);

#endif // UTILS_H
