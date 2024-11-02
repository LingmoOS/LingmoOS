/*
 * liblingmosdk-qtwidgets's Library
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "kbadge.h"
#include "themeController.h"
#include <QPainter>
#include <QRect>
#include <QFont>
#include <QFontMetrics>
#include <QApplication>

namespace kdk
{
class KBadgePrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KBadge)

public:
    KBadgePrivate(KBadge* parent);

private:
    KBadge* q_ptr;
    QColor m_color;
    int m_value;
    int m_fontSize;
    bool m_isShowValue;
    bool m_useCustomColor;
};

KBadgePrivate::KBadgePrivate(KBadge *parent)
    :q_ptr(parent)
{
    Q_Q(KBadge);
    m_value = -1;
    m_color = q->palette().color(QPalette::Highlight);
    m_fontSize = 14;
    m_isShowValue = true;
    m_useCustomColor=false;
    setParent(parent);
}

KBadge::KBadge(QWidget *parent)
    :QWidget(parent),
      d_ptr(new KBadgePrivate(this))
{
    Q_D(KBadge);
    setMinimumSize(40,30);

}

void KBadge::setValue(int value)
{
    Q_D(KBadge);
    d->m_value = value;
}

void KBadge::setValueVisiable(bool flag)
{
    Q_D(KBadge);
    d->m_isShowValue = flag;
}

bool KBadge::isValueVisiable() const
{
    Q_D(const KBadge);
    return d->m_isShowValue;
}

int KBadge::value()
{
    Q_D(KBadge);
    return d->m_value;
}

QColor KBadge::color()
{
    Q_D(KBadge);
    return d->m_color;
}

void KBadge::setColor(const QColor &color)
{
    Q_D(KBadge);
    d->m_useCustomColor=true;
    d->m_color = color;
}

int KBadge::fontSize()
{
    Q_D(KBadge);
    return d->m_fontSize;
}

void KBadge::setFontSize(int size)
{
    Q_D(KBadge);
    if(size<1 ||size >100)
        return;
    d->m_fontSize = size;
}

void KBadge::paintEvent(QPaintEvent *event)
{
    Q_D(KBadge);
    QFont font(QApplication::font());
    font.setPixelSize(d->m_fontSize);
    QFontMetrics fm(font);
    int height = fm.height();
    int width;
    if(d->m_value <1 ||!d->m_isShowValue)
    {
        width = 10;
        height = 10;
    }
    else if(d->m_value >= 1 && d->m_value < 1000)
    {
        width = fm.width(QString::number(d->m_value)) + 10;
        width = width > height ? width:height;
    }
    else
    {
        width = fm.width(QString::number(999)) + 10;
        width = width > height ? width:height;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    if(d->m_useCustomColor)
        painter.setBrush(d->m_color);
    else
        painter.setBrush(palette().color(QPalette::Highlight));
    QRect tmpRect(rect().center().x()-width/2,rect().center().y()-height/2,width,height);
    painter.drawRoundedRect(tmpRect,height/2,height/2);
    //文字颜色固定
    painter.setPen(QWidget::palette().color(QPalette::Light));
    if(d->m_value >= 1 && d->m_value<1000 && d->m_isShowValue)
    {
        QFont font(QApplication::font());
        font.setPixelSize(d->m_fontSize);
        painter.setFont(font);
        painter.drawText(tmpRect,Qt::AlignCenter,QString::number(d->m_value));
    }

    if(d->m_value >= 1000 && d->m_value < INTMAX_MAX && d->m_isShowValue)
    {
        painter.setBrush(QWidget::palette().color(QPalette::Light));
        QPointF pointf(rect().center().x(),rect().center().y());
        painter.drawEllipse(pointf,qreal(1.5),qreal(1.5));
        QPointF leff(pointf.x()-10,pointf.y());
        QPointF rightf(pointf.x()+10,pointf.y());
        painter.drawEllipse(leff,qreal(1.5),qreal(1.5));
        painter.drawEllipse(rightf,qreal(1.5),qreal(1.5));
    }
}

void KBadge::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    repaint();
}

}
#include "kbadge.moc"
#include "moc_kbadge.cpp"

