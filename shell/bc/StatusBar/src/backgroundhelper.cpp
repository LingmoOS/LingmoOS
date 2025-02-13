/*
 * Copyright (C) 2024 - 2022 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "backgroundhelper.h"

#include <QApplication>
#include <QDebug>
#include <QPixmap>
#include <QPixmapCache>
#include <QScreen>
#include <QRgb>

BackgroundHelper::BackgroundHelper(QObject *parent)
    : QObject(parent)
    , m_statusBarHeight(25 / qApp->devicePixelRatio())
    , m_type(0)
{
    onPrimaryScreenChanged();
    connect(qApp, &QApplication::primaryScreenChanged, this, &BackgroundHelper::onPrimaryScreenChanged);
}

void BackgroundHelper::setColor(QColor c)
{
    m_color = c;
    m_type = 1;

    bool isDark = (c.red() * 0.299 +
                   c.green() * 0.587 +
                   c.blue() * 0.114) < 186;

    emit newColor(c, isDark);
}

void BackgroundHelper::setBackgound(const QString &fileName)
{
    m_wallpaper = fileName;
    m_type = 0;

    QImage img(fileName);

    QSize screenSize = qApp->primaryScreen()->geometry().size();
    img = img.scaled(screenSize.width(), screenSize.height());
    img = img.copy(QRect(0, 0, screenSize.width(), m_statusBarHeight));

    QSize size(img.size());
    img = img.scaledToWidth(size.width() * 0.8);
    size = img.size();

    long long sumR = 0, sumG = 0, sumB = 0;
    int measureArea = size.width() * size.height();

    for (int y = 0; y < size.height(); ++y) {
        QRgb *line = (QRgb *)img.scanLine(y);

        for (int x = 0; x < size.width(); ++x) {
            sumR += qRed(line[x]);
            sumG += qGreen(line[x]);
            sumB += qBlue(line[x]);
        }
    }

    sumR /= measureArea;
    sumG /= measureArea;
    sumB /= measureArea;

    QColor c = QColor(sumR, sumG, sumB);
    QColor textColor = (sumR * 0.299 +
                        sumG * 0.587 +
                        sumB * 0.114) > 186 ? "#000000" : "#FFFFFF";

    qDebug() << c << textColor;

    emit newColor(c, textColor == "#FFFFFF");

    // clear cache.
    QPixmapCache::clear();
}

void BackgroundHelper::onPrimaryScreenChanged()
{
    disconnect(qApp->primaryScreen());

    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &BackgroundHelper::onChanged);
    connect(qApp->primaryScreen(), &QScreen::virtualGeometryChanged, this, &BackgroundHelper::onChanged);
}

void BackgroundHelper::onChanged()
{
    switch (m_type) {
    case 0:
        setBackgound(m_wallpaper);
        break;
    case 1:
        setColor(m_color);
        break;
    default:
        break;
    }
}
