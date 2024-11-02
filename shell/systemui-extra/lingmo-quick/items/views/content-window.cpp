/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "content-window.h"
#include "lingmo/screen-area-utils.h"

#include <KWindowSystem>
#include <KWindowEffects>
#include <QQuickItem>
#include <QScreen>
#include <QRect>

namespace LingmoUIQuick {

// ====== ContentWindow ======
ContentWindow::ContentWindow(QWindow *parent) : LINGMOWindow(parent), m_margin(new Margin(this))
{
    setColor(QColor(Qt::transparent));
    setSkipTaskBar(true);
    setSkipSwitcher(true);
    setEnableBlurEffect(true);

    connect(m_margin, &Margin::leftChanged, this, &ContentWindow::updateLocation);
    connect(m_margin, &Margin::topChanged, this, &ContentWindow::updateLocation);
    connect(m_margin, &Margin::rightChanged, this, &ContentWindow::updateLocation);
    connect(m_margin, &Margin::bottomChanged, this, &ContentWindow::updateLocation);
}

QScreen *ContentWindow::getScreen() const
{
    return screen();
}

void ContentWindow::setWindowScreen(QScreen *screen)
{
    if (screen == LINGMOWindow::screen()) {
        return;
    }

    setScreen(screen);
    updateLocation();
    Q_EMIT screenChanged();
}
bool ContentWindow::enableBlurEffect() const
{
    return m_enableBlurEffect;
}

void ContentWindow::setEnableBlurEffect(bool enable)
{
    if (m_enableBlurEffect == enable) {
        return;
    }

    m_enableBlurEffect = enable;

    // TODO: 基于wlcom设置窗口毛玻璃效果
    KWindowEffects::enableBlurBehind(this, enable);
}

QQuickItem *ContentWindow::content() const
{
    return m_content;
}

void ContentWindow::setContent(QQuickItem *content)
{
    if (m_content == content) {
        return;
    }

    if (m_content) {
        m_content->setParentItem(nullptr);
        m_content->disconnect(this, nullptr);
    }

    m_content = content;
    if (m_content) {
        m_content->setParentItem(contentItem());

        onContentWidthChanged();
        onContentHeightChanged();
        updateLocation();

        connect(m_content, &QQuickItem::widthChanged, this, &ContentWindow::onContentWidthChanged);
        connect(m_content, &QQuickItem::heightChanged, this, &ContentWindow::onContentHeightChanged);
    }

    emit contentChanged();
}

Types::Pos ContentWindow::getPos() const
{
    return m_pos;
}

void ContentWindow::setPos(Types::Pos pos)
{
    if (m_pos == pos) {
        return;
    }

    m_pos = pos;
    updateLocation();
    emit positionChanged();
}

Margin *ContentWindow::margin() const
{
    return m_margin;
}

bool ContentWindow::useAvailableGeometry() const
{
    return m_useAvailableGeometry;
}

void ContentWindow::setUseAvailableGeometry(bool use)
{
    if (m_useAvailableGeometry == use) {
        return;
    }

    m_useAvailableGeometry = use;
    Q_EMIT useAvailableGeometryChanged();
    updateLocation();
    Q_EMIT positionChanged();
}

void ContentWindow::onContentWidthChanged()
{
    setWidth(m_content->width());
    updateLocation();
}

void ContentWindow::onContentHeightChanged()
{
    setHeight(m_content->height());
    updateLocation();
}

void ContentWindow::updateLocation()
{
    if (m_pos == Types::NoPosition) {
        return;
    }

    QScreen *currentScreen = getScreen();
    if (!currentScreen) {
        if (parent()) {
            currentScreen = parent()->screen();
        }

        if (!currentScreen) {
            return;
        }
    }

    QRect ar = m_useAvailableGeometry? ScreenAreaUtils::instance()->getAvailableGeometry(currentScreen) : currentScreen->geometry();
    int nx = 0, ny = 0;
    switch (m_pos) {
        case Types::TopLeft:
            nx = ar.left() + m_margin->left();
            ny = ar.top() + m_margin->top();
            break;
        case Types::TopRight:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.top() + m_margin->top();
            break;
        case Types::BottomRight:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        case Types::BottomLeft:
            nx = ar.left() + m_margin->left();
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        case Types::Left:
        case Types::Top:
            nx = ar.left() + m_margin->left();
            ny = ar.top() + m_margin->top();
            break;
        case Types::Right:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.top() + m_margin->top();
            break;
        case Types::Bottom:
            nx = ar.left() + m_margin->left();
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        case Types::Center:
            nx = ar.left() + ar.width()/2 - width()/2;
            nx = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::LeftCenter:
            nx = ar.left() + m_margin->left();
            ny = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::TopCenter:
            nx = ar.left() + ar.width()/2 - width()/2;
            ny = ar.top() + m_margin->top();
            break;
        case Types::RightCenter:
            nx = ar.right() - width() - m_margin->right();
            ny = ar.top() + ar.height()/2 - height()/2;
            break;
        case Types::BottomCenter:
            nx = ar.left() + ar.width()/2 - width()/2;
            ny = ar.bottom() - height() - m_margin->bottom();
            break;
        default:
            nx = ar.left();
            ny = ar.top();
            break;
    }

    if (nx < ar.left()) {
        nx = ar.left();
    } else if ((nx + width()) > ar.right()) {
        nx = ar.right() - width();
    }

    if (ny < ar.top()) {
        ny = ar.top();
    } else if ((ny + height()) > ar.bottom()) {
        ny = ar.bottom() - height();
    }

    setWindowPosition({nx, ny});
}

bool ContentWindow::event(QEvent *event)
{
    if (event->type() == QEvent::Show) {
        KWindowEffects::enableBlurBehind(this, m_enableBlurEffect);
    }

    return LINGMOWindow::event(event);
}

} // LingmoUIQuick
