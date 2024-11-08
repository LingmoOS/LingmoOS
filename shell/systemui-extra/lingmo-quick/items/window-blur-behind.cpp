/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include <QPainterPath>
#include <KWindowEffects>
#include <QQuickWindow>
#include "window-blur-behind.h"

namespace LingmoUIQuick {
RectRegion::RectRegion(QObject *parent) : QObject(parent)
{
}

void RectRegion::setX(int x)
{
    if(m_x == x) {
        return;
    }
    m_x = x;
    Q_EMIT xChanged();
}

int RectRegion::x() const
{
    return m_x;
}

void RectRegion::setY(int y)
{
    if(m_y == y) {
        return;
    }
    m_y = y;
    Q_EMIT yChanged();
}

int RectRegion::y() const
{
    return m_y;
}

void RectRegion::setWidth(int width)
{
    if(m_width == width) {
        return;
    }
    m_width = width;
    Q_EMIT widthChanged();
}

int RectRegion::width() const
{
    return m_width;
}

void RectRegion::setHeight(int height)
{
    if(m_height == height) {
        return;
    }
    m_height = height;
    Q_EMIT heightChanged();
}

int RectRegion::height() const
{
    return m_height;
}

void RectRegion::setRadius(int radius)
{
    if(m_radius == radius) {
        return;
    }
    m_radius = radius;
    Q_EMIT radiusChanged();
}

int RectRegion::radius() const
{
    return m_radius;
}

WindowBlurBehind::WindowBlurBehind(QQuickItem *parent) : QQuickItem(parent)
{
}

QQmlListProperty<RectRegion> WindowBlurBehind::rectRegions()
{
    return {this, &m_rects,
            &WindowBlurBehind::appendRect,
            &WindowBlurBehind::rectCount,
            &WindowBlurBehind::rectAt,
            &WindowBlurBehind::clearRect,
            &WindowBlurBehind::replaceRect,
            &WindowBlurBehind::removeLastRect
    };
}

void WindowBlurBehind::setEnable(bool enable)
{
    if(m_enable == enable) {
        return;
    }
    m_enable = enable;
    updateBlurRegion();
    Q_EMIT enableChanged();
}

bool WindowBlurBehind::enable() const
{
    return m_enable;
}

void WindowBlurBehind::updateBlurRegion()
{
    if(!m_window) {
        return;
    }
    if(m_enable) {
        QPainterPath path;
        for (const RectRegion *rect : m_rects) {
            path.addRoundedRect(rect->x(), rect->y(), rect->width(), rect->height(), rect->radius(), rect->radius());
        }
        if(path.isEmpty()) {
            KWindowEffects::enableBlurBehind(m_window, false);
        } else {
            KWindowEffects::enableBlurBehind(m_window, true, QRegion(path.toFillPolygon().toPolygon()));
        }
    } else {
        KWindowEffects::enableBlurBehind(m_window, false);
    }
}

void WindowBlurBehind::appendRect(QQmlListProperty<RectRegion> *list, RectRegion *rect)
{
    if (!rect) {
        return;
    }
    auto wbb = qobject_cast<WindowBlurBehind *>(list->object);
    if (!wbb->m_rects.contains(rect)) {
        wbb->m_rects.append(rect);
        wbb->updateBlurRegion();
    }
}

int WindowBlurBehind::rectCount(QQmlListProperty<RectRegion> *list)
{
    auto wbb = qobject_cast<WindowBlurBehind *>(list->object);
    return wbb->m_rects.size();
}

RectRegion *WindowBlurBehind::rectAt(QQmlListProperty<RectRegion> *list, int index)
{
    auto wbb = qobject_cast<WindowBlurBehind *>(list->object);
    if (index < 0 || index >= wbb->m_rects.size()) {
        return nullptr;
    }

    return wbb->m_rects.at(index);
}

void WindowBlurBehind::clearRect(QQmlListProperty<RectRegion> *list)
{
    auto wbb = qobject_cast<WindowBlurBehind *>(list->object);
    wbb->m_rects.clear();
    wbb->updateBlurRegion();
}

void WindowBlurBehind::replaceRect(QQmlListProperty<RectRegion> *list, int index, RectRegion *rect)
{
    if (!rect) {
        return;
    }

    auto wbb = qobject_cast<WindowBlurBehind *>(list->object);
    if (index < 0 || index >= wbb->m_rects.size()) {
        return;
    }
    wbb->m_rects.replace(index, rect);
    wbb->updateBlurRegion();
}

void WindowBlurBehind::removeLastRect(QQmlListProperty<RectRegion> *list)
{
    auto wbb = qobject_cast<WindowBlurBehind *>(list->object);
    if (wbb->m_rects.isEmpty()) {
        return;
    }
    wbb->m_rects.removeLast();
    wbb->updateBlurRegion();
}

void WindowBlurBehind::addRectRegion(RectRegion *rect)
{
    if (!rect) {
        return;
    }
    if (!m_rects.contains(rect)) {
        m_rects.append(rect);
        updateBlurRegion();
    }
}

void WindowBlurBehind::setWindow(QWindow *window)
{
    if(m_window == window) {
        return;
    }
    m_window = window;
    Q_EMIT windowChanged();
    updateBlurRegion();
}

QWindow *WindowBlurBehind::window()
{
    return m_window;
}

void WindowBlurBehind::clearRectRegion()
{
    m_rects.clear();
    updateBlurRegion();
}

} // LingmoUIQuick
