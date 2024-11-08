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

#include "kbreadcrumb.h"
#include <QPainter>
#include <QStyle>
#include <QStyleOptionTab>
#include <QFontMetrics>
#include <QApplication>
#include <QDebug>
#include "themeController.h"

namespace kdk
{
class KBreadCrumbPrivate:public QObject,public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KBreadCrumb)

public:
    KBreadCrumbPrivate(KBreadCrumb*parent);

private:
    KBreadCrumb* q_ptr;
    QIcon m_icon;
    bool m_isFlat;
};

KBreadCrumb::KBreadCrumb(QWidget *parent)
    :QTabBar(parent),
      d_ptr(new KBreadCrumbPrivate(this))
{

}

void KBreadCrumb::setIcon(const QIcon &icon)
{
    Q_D(KBreadCrumb);
    d->m_icon = icon;
    update();
}

QIcon KBreadCrumb::icon() const
{
    Q_D(const KBreadCrumb);
    return d->m_icon;
}

bool KBreadCrumb::isFlat() const
{
    Q_D(const KBreadCrumb);
    return d->m_isFlat;
}

void KBreadCrumb::setFlat(bool flat)
{
    Q_D(KBreadCrumb);
    d->m_isFlat = flat;
    update();
}

QSize KBreadCrumb::tabSizeHint(int index) const
{
    Q_D(const KBreadCrumb);
    QSize size = QTabBar::tabSizeHint(index);
    QFont font(QApplication::font());
    QFontMetrics fm(font);
    int width = fm.width(tabText(index));
    size.setWidth(width+40);
    if(index == 0 && !d->m_icon.isNull())
        size.setWidth(size.width()+50);
    return size;
}

void KBreadCrumb::paintEvent(QPaintEvent *event)
{
    Q_D(const KBreadCrumb);
    QColor highLightColor = palette().color(QPalette::Highlight);
    QColor baseColor;
    QColor focusColor;
    if(ThemeController::themeMode() == ThemeFlag::DarkTheme)
    {
        if(d->m_isFlat)
            baseColor = "#D9D9D9";
        else
            baseColor = "#47474A";
        focusColor = "#D9D9D9";
    }
    else
    {
        if(d->m_isFlat)
            baseColor = "#262626";
        else
            baseColor = "#B3B3B3";
        focusColor = "#262626";
    }
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    for(int i = 0 ; i < count(); i++)
    {
        QRect rc = tabRect(i);
        QStyleOptionTab option;
        initStyleOption(&option, i);
        p.setPen(Qt::NoPen);
        p.drawRect(rc);
        if(QStyle::State_MouseOver & option.state)
            p.setPen(highLightColor);
        else if(QStyle::State_Selected & option.state)
            p.setPen(focusColor);
        else
            p.setPen(baseColor);
        p.setBrush(Qt::NoBrush);

        if(layoutDirection() == Qt::LeftToRight)
        {
            if(i == 0 && !d->m_icon.isNull())
            {
                p.drawPixmap(rc.left()+10,(rc.height()-24)/2,24,24,
                             d->m_icon.pixmap(24,24));
                p.drawText(rc.adjusted(30,0,-24,0),Qt::AlignCenter,tabText(i));
            }
            else
                p.drawText(rc.adjusted(0,0,-24,0),Qt::AlignCenter,tabText(i));
            if(i != count()-1)
                p.drawPixmap(rc.right()-24,(rc.height()-16)/2,16,16,
                             ThemeController::drawColoredPixmap(QIcon::fromTheme("lingmo-end-symbolic").pixmap(16,16),baseColor));
        }
        else
        {
            if(i == 0 && !d->m_icon.isNull())
            {
                p.drawPixmap(rc.right()-34,(rc.height()-24)/2,24,24,
                             d->m_icon.pixmap(24,24));
                p.drawText(rc.adjusted(24,0,-30,0),Qt::AlignCenter,tabText(i));
            }
            else
                p.drawText(rc.adjusted(24,0,0,0),Qt::AlignCenter,tabText(i));
            if(i != count()-1)
                p.drawPixmap(rc.left()+8,(rc.height()-16)/2,16,16,
                             ThemeController::drawColoredPixmap(QIcon::fromTheme("lingmo-start-symbolic").pixmap(16,16),baseColor));
        }
    }
}


KBreadCrumbPrivate::KBreadCrumbPrivate(KBreadCrumb *parent)
    :q_ptr(parent),
      m_isFlat(true)
{
}

}

#include "kbreadcrumb.moc"
#include "moc_kbreadcrumb.cpp"
