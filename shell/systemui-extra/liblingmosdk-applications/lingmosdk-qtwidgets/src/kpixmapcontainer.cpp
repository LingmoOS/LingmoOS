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

#include "kpixmapcontainer.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QDebug>

const static int margin = 10;
namespace kdk
{

class KPixmapContainerPrivate:public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KPixmapContainer)

public:
    KPixmapContainerPrivate(KPixmapContainer*parent);

private:
    KPixmapContainer* q_ptr;
    QPixmap m_pixmap;
    QColor m_color;
    int m_fontSize;
    int m_value;
    bool m_isShowValue;

};

KPixmapContainer::KPixmapContainer(QWidget *parent)
    :QWidget(parent),
      d_ptr(new KPixmapContainerPrivate(this))
{
    Q_D(KPixmapContainer);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}

int KPixmapContainer::value() const
{
    Q_D(const KPixmapContainer);
    return d->m_value;
}

void KPixmapContainer::setValue(int value)
{
    Q_D(KPixmapContainer);
    d->m_value = value;
}

void KPixmapContainer::setValueVisiable(bool flag)
{
    Q_D(KPixmapContainer);
    d->m_isShowValue = flag;
}

bool KPixmapContainer::isValueVisiable() const
{
    Q_D(const KPixmapContainer);
    return d->m_isShowValue;
}

void KPixmapContainer::setPixmap(const QPixmap& pixmap)
{
    Q_D(KPixmapContainer);
    d->m_pixmap = pixmap;
    QSize size = QSize(d->m_pixmap.size().width()+20, d->m_pixmap.size().height()+20);
    this->setFixedSize(size);
    update();
}

QPixmap KPixmapContainer::pixmap() const
{
    Q_D(const KPixmapContainer);
    if(!d->m_pixmap.isNull())
        return d->m_pixmap;
    else
        return QPixmap();
}

void KPixmapContainer::clearValue()
{
    Q_D(KPixmapContainer);
    d->m_value = 0;
}

QColor KPixmapContainer::color()
{
    Q_D(KPixmapContainer);
    return d->m_color;
}

void KPixmapContainer::setColor(const QColor &color)
{
    Q_D(KPixmapContainer);
    d->m_color = color;
}

int KPixmapContainer::fontSize()
{
    Q_D(KPixmapContainer);
    return d->m_fontSize;
}

void KPixmapContainer::setFontSize(int size)
{
    Q_D(KPixmapContainer);
    if(size<1 ||size >100)
        return;
    d->m_fontSize = size;
    update();
}

void KPixmapContainer::paintEvent(QPaintEvent *event)
{
    Q_D(KPixmapContainer);
    QWidget::paintEvent(event);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.drawRect(this->rect());

    QFont font(QApplication::font());
    font.setPixelSize(d->m_fontSize);
    QFontMetrics fm(font);
    int height = fm.height();
    int width;
    if(d->m_value <1 || !d->m_isShowValue)
    {
        width = 10;
        height = 10;
    }
    else if(d->m_value >= 1 && d->m_value < 1000)
    {
        width = fm.width(QString::number(d->m_value));
        width = width > height ? width:height;
    }
    else
    {
        width = fm.width(QString::number(999));
        width = width > height ? width:height;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    d->m_color = palette().color(QPalette::Highlight);

    painter.setBrush(d->m_color);
    painter.drawPixmap(margin,margin,d->m_pixmap.width(),d->m_pixmap.height(),d->m_pixmap);
    QRect tmpRect(this->rect().topRight().x()-width/2-margin,this->rect().topRight().y()-height/2+margin,width,height);
    painter.drawRoundedRect(tmpRect,height/2,height/2);
    painter.setPen(QColor(255,255,255));
    if(d->m_value >= 1 && d->m_value<1000 && d->m_isShowValue)
    {
        QFont font(QApplication::font());
        font.setPixelSize(d->m_fontSize);
        painter.setFont(font);
        painter.drawText(tmpRect,Qt::AlignCenter,QString::number(d->m_value));
    }

    if(d->m_value >= 1000 && d->m_value < INTMAX_MAX && d->m_isShowValue)
    {
        painter.setBrush(QColor(255,255,255));
        QPointF pointf(tmpRect.center().x(),tmpRect.center().y());
        painter.drawEllipse(pointf,qreal(1.0),qreal(1.0));
        QPointF leff(pointf.x()-5,pointf.y());
        QPointF rightf(pointf.x()+5,pointf.y());
        painter.drawEllipse(leff,qreal(1.0),qreal(1.0));
        painter.drawEllipse(rightf,qreal(1.0),qreal(1.0));
    }
}

KPixmapContainerPrivate::KPixmapContainerPrivate(KPixmapContainer *parent)
    :q_ptr(parent)
{
    Q_Q(KPixmapContainer);
    m_value = -1;
    m_isShowValue = true;
    m_color = QColor(55,144,250);
    m_fontSize = 10;
    setParent(parent);
}

}

#include "kpixmapcontainer.moc"
#include "moc_kpixmapcontainer.cpp"

