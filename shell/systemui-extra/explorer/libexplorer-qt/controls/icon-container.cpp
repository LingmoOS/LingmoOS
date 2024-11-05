/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "icon-container.h"
#include <QPaintEvent>
#include <QPainter>

#include <QStyleOptionButton>

#include <QGraphicsDropShadowEffect>

#include <QApplication>

using namespace Peony;

IconContainer::IconContainer(QWidget *parent) : QPushButton(parent)
{
    setEnabled(true);
    setCheckable(false);
    setDefault(true);
    setFlat(true);

    m_style = new IconContainerStyle;
    setStyle(m_style);

    //fix push button use as icon caused color issues
    this->setProperty("isIcon", true);
    setAttribute(Qt::WA_TranslucentBackground);

    auto shadowEffect = new QGraphicsDropShadowEffect;
    shadowEffect->setBlurRadius(20);
    shadowEffect->setOffset(0, 0);
    setGraphicsEffect(shadowEffect);
}

IconContainer::~IconContainer()
{
    m_style->deleteLater();
}

void IconContainer::paintEvent(QPaintEvent *e)
{
    //QPainter p(this);
    //p.fillRect(this->rect(), this->palette().base());
    //p.setPen(this->palette().dark().color());
    //p.drawRect(this->rect().adjusted(0, 0, -1, -1));
    QPushButton::paintEvent(e);
}

IconContainerStyle::IconContainerStyle() : QProxyStyle()
{

}

void IconContainerStyle::drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if(element == CE_PushButton)
    {
        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            QStyleOptionButton subopt = *button;
            subopt.palette.setColor(QPalette::Highlight, Qt::transparent);
            subopt.rect = proxy()->subElementRect(SE_PushButtonContents, option, widget);
            proxy()->drawControl(CE_PushButtonLabel, &subopt, painter, widget);
            return;
        }
    }
    return qApp->style()->drawControl(element, option, painter, widget);
}
