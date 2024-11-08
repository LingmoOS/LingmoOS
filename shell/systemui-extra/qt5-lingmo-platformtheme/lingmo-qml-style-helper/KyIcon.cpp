/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */

#include "KyIcon.h"
#include <QStyleOption>
#include <QStyle>
#include <QPainter>
#include <QApplication>
#include "effects/highlight-effect.h"
#include <QGSettings/QGSettings>

QStyle *KyIcon::style()
{
    return qApp->style();
}

KyIcon::KyIcon(QQuickPaintedItem *parent)
    : QQuickPaintedItem(parent),
      m_hover(false),
      m_selected(false),
      m_focus(false),
      m_active(true),
      m_sunken(false),
      m_on(false),
      m_icontype("default")
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings* styleSettings = new QGSettings("org.lingmo.style", QByteArray(), this);
        connect(styleSettings, &QGSettings::changed, this, [&](){
            emit hoverChanged();
            emit selectedChanged();
            emit hasFocusChanged();
            emit activeChanged();
            emit sunkenChanged();
            emit onChanged();
            emit icontypeChanged();
            emit iconNameChanged();
            update();
        });
    }
    connect(this, &KyIcon::iconNameChanged, this, &KyIcon::updateItem);
    connect(this, &KyIcon::hoverChanged, this, &KyIcon::updateItem);
    connect(this, &KyIcon::selectedChanged, this, &KyIcon::updateItem);
    connect(this, &KyIcon::hasFocusChanged, this, &KyIcon::updateItem);
    connect(this, &KyIcon::activeChanged, this, &KyIcon::updateItem);
    connect(this, &KyIcon::sunkenChanged, this, &KyIcon::updateItem);
    connect(this, &KyIcon::onChanged, this, &KyIcon::updateItem);
    connect(this, &KyIcon::icontypeChanged, this, &KyIcon::updateItem);


}


void KyIcon::setIcon(const QIcon &icon)
{
    m_icon = icon;
}

void KyIcon::setIconName(const QString &iconName)
{
    m_iconName = iconName;
    if(!QIcon::hasThemeIcon(m_iconName))
    {
        m_icon = QIcon();
        qWarning() << "未找到名为 " << m_iconName << " 的图标!";
        return;
    }
    m_icon = QIcon::fromTheme(m_iconName);
    emit iconNameChanged();
}

void KyIcon::paint(QPainter *painter)
{

    if(m_icon.isNull())
        return;
    QWidget wid;
    QStyleOption opt;
    opt.state = {};
    if (isEnabled()) {
        opt.state |= QStyle::State_Enabled;
    }
    if (m_hover)
        opt.state |= QStyle::State_MouseOver;
    if (m_selected)
        opt.state |= QStyle::State_Selected;
    if (m_focus)
        opt.state |= QStyle::State_HasFocus;
    if (m_active)
        opt.state |= QStyle::State_Active;
    if (m_sunken)
        opt.state |= QStyle::State_Sunken;
    if (m_on)
        opt.state |= QStyle::State_On;

    QPixmap pixmap = m_icon.pixmap(QSize(width(), height()));

    if (m_icontype == "ordinary") {
        pixmap = HighLightEffect::ordinaryGeneratePixmap(pixmap, &opt, &wid); /* 主题切换做处理*/
    }
    if (m_icontype == "hover") {
        pixmap = HighLightEffect::hoverGeneratePixmap(pixmap, &opt, &wid);   /* 选中点击做处理*/
    }
    if (m_icontype == "filledSymbolicColor") {
        pixmap = HighLightEffect::filledSymbolicColoredGeneratePixmap(pixmap, &opt, &wid); /* 非纯色图标主题切换且选中点击做处理*/
    }
    else if (m_icontype == "default") {
        pixmap = HighLightEffect::bothOrdinaryAndHoverGeneratePixmap(pixmap, &opt, &wid);  /* 主题切换且选中点击做处理*/
    }

    KyIcon::style()->drawItemPixmap(painter, boundingRect().toRect(), Qt::AlignCenter,pixmap);

}
