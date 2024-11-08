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
#include "utils.h"
#include "my_qt.h"

std::vector<QRect> GetScreenGeometries() {
    std::vector<QRect> screen_geometries;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    for(QScreen *screen :  QApplication::screens()) {
        QRect geometry = screen->geometry();
        qreal ratio = screen->devicePixelRatio();
        screen_geometries.emplace_back(geometry.x(), geometry.y(), lrint((qreal) geometry.width() * ratio), lrint((qreal) geometry.height() * ratio));
    }
#else
    for(int i = 0; i < QApplication::desktop()->screenCount(); ++i) {
        screen_geometries.push_back(QApplication::desktop()->screenGeometry(i));
    }
#endif
    return screen_geometries;
}

QRect CombineScreenGeometries(const std::vector<QRect>& screen_geometries) {
    QRect combined_geometry;
    for(const QRect &geometry : screen_geometries) {
        combined_geometry |= geometry;
    }
    return combined_geometry;
}

// This does some sanity checking on the rubber band rectangle before creating it.
// Rubber bands with width or height zero or extremely large appear to cause problems.
QRect ValidateRubberBandRectangle(QRect rect) {
    std::vector<QRect> screen_geometries = GetScreenGeometries();
    QRect combined_geometry = CombineScreenGeometries(screen_geometries);
    return rect.normalized() & combined_geometry.adjusted(-10, -10, 10, 10);
}

QRect MapToLogicalCoordinates(const QRect& rect) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    for(QScreen *screen :  QApplication::screens()) {
        QRect geometry = screen->geometry();
        qreal ratio = screen->devicePixelRatio();
        QRect physical_geometry(geometry.x(), geometry.y(), lrint((qreal) geometry.width() * ratio), lrint((qreal) geometry.height() * ratio));
        if(physical_geometry.contains(rect.center())) {
            return QRect(
                        geometry.x() + lrint((qreal) (rect.x() - physical_geometry.x()) / ratio - 0.4999),
                        geometry.y() + lrint((qreal) (rect.y() - physical_geometry.y()) / ratio - 0.4999),
                        lrint((qreal) rect.width() / ratio - 0.4999),
                        lrint((qreal) rect.height() / ratio - 0.4999));
        }
    }
#endif
    return rect;
};

QString GetNewSegmentFile(const QString& file, bool add_timestamp) {
    QFileInfo fi(file);
    QDateTime now = QDateTime::currentDateTime();
    QString newfile;
    unsigned int counter = 0;
    do {
        ++counter;
        newfile = fi.path() + "/" + fi.completeBaseName();
        if(add_timestamp)
            newfile += "-" + now.toString("yyyy-MM-dd_hh.mm.ss");
        if(counter != 1)
            newfile += "-(" + QString::number(counter) + ")";
        if(!fi.suffix().isEmpty())
            newfile += "." + fi.suffix();
    } while(QFileInfo(newfile).exists());
    return newfile;
}

QString ReadableTime(int64_t time_micro) {
    unsigned int time = (time_micro + 500000) / 1000000;
    return QString("%1:%2:%3")
            .arg(time / 3600)
            .arg((time / 60) % 60, 2, 10, QLatin1Char('0'))
            .arg(time % 60, 2, 10, QLatin1Char('0'));
}

QString ReadableWidthHeight(unsigned int width, unsigned int height) {
    if(width == 0 && height == 0)
        return "?";
    return QString::number(width) + "x" + QString::number(height);
}

QString ReadableSizeIEC(uint64_t size, const QString& suffix) {
   if(size < (uint64_t) 10 * 1024)
       return QString::number(size) + " " + suffix;
   if(size < (uint64_t) 10 * 1024 * 1024)
       return QString::number((size + 512) / 1024) + " Ki" + suffix;
   if(size < (uint64_t) 10 * 1024 * 1024 * 1024)
       return QString::number((size / 1024 + 512) / 1024) + " Mi" + suffix;
   return QString::number((size / (1024 * 1024) + 512) / 1024) + " Gi" + suffix;
}

QString ReadableSizeSI(uint64_t size, const QString& suffix) {
    if(size < (uint64_t) 10 * 1000)
        return QString::number(size) + " " + suffix;
    if(size < (uint64_t) 10 * 1000 * 1000)
        return QString::number((size + 500) / 1000) + " k" + suffix;
    if(size < (uint64_t) 10 * 1000 * 1000 * 1000)
        return QString::number((size / 1000 + 500) / 1000) + " M" + suffix;
    return QString::number((size / (1000 * 1000) + 512) / 1024) + " G" + suffix;
}
