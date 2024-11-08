/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
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
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "theme-icon.h"
#include "icon-helper.h"

#include <QUrl>
#include <QPainter>
#include <QFile>
#include <QImageReader>
#include <QDebug>
#include <QImage>
#include <QtMath>
#include <QBitmap>
#include <QPainterPath>
#include <QGuiApplication>

#define COLOR_DIFFERENCE 10

namespace LingmoUIQuick {
QColor ThemeIcon::symbolicColor = QColor(38, 38, 38);

ThemeIcon::ThemeIcon(QQuickItem *parent) : QQuickPaintedItem(parent), m_mode(Icon::Normal)
{
    m_keepAspectRatio = true;
}

void ThemeIcon::paint(QPainter *painter)
{
    //默认居中绘制
    QRectF rect({0, 0}, size().toSize());
    QPixmap target;
    QRectF sourceRect;
    QRectF targetRect;
    if (m_keepAspectRatio) {
        if (!m_sourceSize.isEmpty()) {
            target = m_rawIcon.pixmap(m_sourceSize);
            target.setDevicePixelRatio(qApp->devicePixelRatio());
            qreal widthHeightRatio = rect.width()/rect.height();
            qreal sourceWidthHeightRatio = m_sourceSize.width() * 1.0/m_sourceSize.height();
            if (sourceWidthHeightRatio > widthHeightRatio) {
                targetRect.setWidth(rect.width());
                targetRect.setHeight(rect.width()/sourceWidthHeightRatio);
                targetRect.moveCenter(rect.center());
            } else {
                targetRect.setHeight(rect.height());
                targetRect.setWidth(rect.height()*sourceWidthHeightRatio);
                targetRect.moveCenter(rect.center());
            }
        } else {
            int extent = qMin(rect.width(), rect.height());
            target = m_rawIcon.pixmap(QSize(extent, extent));
            target.setDevicePixelRatio(qApp->devicePixelRatio());
            targetRect = target.rect();
            targetRect.moveCenter(rect.center());
            QPointF center = targetRect.center();
            targetRect.setSize(targetRect.size()/qApp->devicePixelRatio());
            targetRect.moveCenter(center);
        }
    } else {
        target = m_rawIcon.pixmap(size().toSize());
        target.setDevicePixelRatio(qApp->devicePixelRatio());
        targetRect = rect;
        QPointF center = targetRect.center();
        targetRect.setSize(targetRect.size()/qApp->devicePixelRatio());
        targetRect.moveCenter(center);
    }
    sourceRect = target.rect();

    painter->save();
    //抗锯齿,平滑过渡
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (m_mode & Icon::Disabled) {
        QPainter p(&target);
        p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(target.rect(), Theme::instance()->color(Theme::ButtonText, Theme::Disabled));

    } else if (m_mode & Icon::Highlight) {
        bool isPureColor = true;
        if(!m_mode.testFlag(Icon::ForceHighlight)) {
            isPureColor = ThemeIcon::isPixmapPureColor(target);
        }
        if (isPureColor) {
            QPainter p(&target);
            p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(target.rect(), Theme::instance()->color(Theme::HighlightedText));
        }
    }

    if (m_radius > 0) {
        auto radius = qMin(m_radius * 1.0, qMin((rect.height() / 2), (rect.width() / 2)));
        QPainterPath path;
        path.addRoundedRect(rect, radius, radius);
        painter->setClipPath(path);
    }

    painter->drawPixmap(targetRect, target, sourceRect);
    painter->restore();
}

QVariant ThemeIcon::getSource()
{
    return m_source;
}

void ThemeIcon::setSource(const QVariant &source)
{
    if (m_source == source) {
        return;
    }
    m_source = source;
    updateRawIcon();
}

QString ThemeIcon::getFallBack()
{
    return m_fallback;
}

void ThemeIcon::setFallBack(const QString &fallback)
{
    if (fallback.isEmpty()) {
        qWarning() << "ThemeIcon: fallback is empty!";
        return;
    }

    if (m_rawIcon.isNull()) {
        setSource(fallback);
    }
}

Icon::Mode ThemeIcon::mode() const
{
    return m_mode;
}

void ThemeIcon::setMode(Icon::Mode mode)
{
    if (m_mode == mode) {
        return;
    }

    m_mode = mode;
    if (m_mode & Icon::AutoHighlight) {
        updateMode();
        connect(Theme::instance(), &Theme::themeNameChanged, this, &ThemeIcon::updateMode);
    } else {
        disconnect(Theme::instance(), nullptr, this, nullptr);
        update();
    }
}

void ThemeIcon::updateMode()
{
    if (Theme::instance()->isDarkTheme()) {
        m_mode |= Icon::Highlight;
    } else {
        m_mode &= ~Icon::Highlight;
    }

    update();
}

//copy from lingmo-platform-theme
bool ThemeIcon::isPixmapPureColor(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        qWarning("pixmap is null!");
        return false;
    }
    QImage image = pixmap.toImage();

    QVector<QColor> vector;
    int total_red = 0;
    int total_green = 0;
    int total_blue = 0;
    bool pure = true;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (image.pixelColor(x, y).alphaF() > 0.3) {
                QColor color = image.pixelColor(x, y);
                vector << color;
                total_red += color.red();
                total_green += color.green();
                total_blue += color.blue();
                int dr = qAbs(color.red() - symbolicColor.red());
                int dg = qAbs(color.green() - symbolicColor.green());
                int db = qAbs(color.blue() - symbolicColor.blue());
                if (dr > COLOR_DIFFERENCE || dg > COLOR_DIFFERENCE || db > COLOR_DIFFERENCE)
                    pure = false;
            }
        }
    }

    if (pure)
        return true;

    qreal squareRoot_red = 0;
    qreal squareRoot_green = 0;
    qreal squareRoot_blue = 0;
    qreal average_red = total_red / vector.count();
    qreal average_green = total_green / vector.count();
    qreal average_blue = total_blue / vector.count();
    for (QColor color : vector) {
        squareRoot_red += (color.red() - average_red) * (color.red() - average_red);
        squareRoot_green += (color.green() - average_green) * (color.green() - average_green);
        squareRoot_blue += (color.blue() - average_blue) * (color.blue() - average_blue);
    }

    qreal arithmeticSquareRoot_red = qSqrt(squareRoot_red / vector.count());
    qreal arithmeticSquareRoot_green = qSqrt(squareRoot_green / vector.count());
    qreal arithmeticSquareRoot_blue = qSqrt(squareRoot_blue / vector.count());

    return arithmeticSquareRoot_red < 2.0 && arithmeticSquareRoot_green < 2.0 && arithmeticSquareRoot_blue < 2.0;
}

QColor ThemeIcon::getSymbolicColor()
{
    return symbolicColor;
}

void ThemeIcon::updateRawIcon()
{
    switch (m_source.userType()) {
    case QMetaType::QPixmap:
        m_rawIcon = QIcon(m_source.value<QPixmap>());
        m_sourceSize = m_source.value<QPixmap>().size();
        break;
    case QMetaType::QImage:
        m_rawIcon = QIcon(QPixmap::fromImage(m_source.value<QImage>()));
        m_sourceSize = m_source.value<QImage>().size();
        break;
    case QMetaType::QBitmap:
        m_rawIcon = QIcon(QPixmap::fromImage(m_source.value<QBitmap>().toImage()));
        m_sourceSize = m_source.value<QBitmap>().size();
        break;
    case QMetaType::QIcon:
        m_rawIcon = m_source.value<QIcon>();
        m_sourceSize = QSize();
        break;
    case QMetaType::QString:
        m_rawIcon = IconHelper::loadIcon(m_source.toString());
        m_sourceSize = QSize();
        break;
    default:
        m_rawIcon = IconHelper::getDefaultIcon();
        m_sourceSize = QSize();
        break;
    }

    if (m_rawIcon.isNull()) {
        QImage image = QImage(QSize(width(), height()), QImage::Format_Alpha8);
        image.fill(Qt::transparent);
        m_rawIcon = QIcon(QPixmap::fromImage(image));
    }
    update();
}

bool ThemeIcon::getKeepAspectRatio() const
{
    return m_keepAspectRatio;
}

void ThemeIcon::setKeepAspectRatio(bool keepAspectRatio)
{
    m_keepAspectRatio = keepAspectRatio;
}

int ThemeIcon::radius()
{
    return m_radius;
}

void ThemeIcon::setRadius(int radius)
{
    m_radius = radius < 0 ? 0 : radius;
}

}
