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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "ktranslucentfloor.h"
#include <QPainter>
#include <QDebug>
#include <QPainter>
#include "kshadowhelper.h"
#include <QRegion>
#include <kwindoweffects.h>
#include <QPainterPath>

namespace kdk {

class KTranslucentFloorPrivate :public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTranslucentFloor)
public:
    KTranslucentFloorPrivate(KTranslucentFloor* parent);

private:
    KTranslucentFloor* q_ptr;
    int m_radius;
    bool m_shadowFlag;
    bool m_enableBlur;
    qreal m_opacity;
};

KTranslucentFloor::KTranslucentFloor(QWidget *parent)
    :QFrame(parent),
      d_ptr(new KTranslucentFloorPrivate(this))
{
    Q_D(KTranslucentFloor);

    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground,true);
}

void KTranslucentFloor::setBorderRadius(int radius)
{
    Q_D(KTranslucentFloor);
    d->m_radius = radius;
    if(shadow())
    {
        effects::KShadowHelper::self()->setWidget(this,d->m_radius);
    }
}

int KTranslucentFloor::borderRadius()
{
    Q_D(KTranslucentFloor);
    return d->m_radius;
}

void KTranslucentFloor::setShadow(bool flag)
{
    Q_D(KTranslucentFloor);
    d->m_shadowFlag = flag;
    if(d->m_shadowFlag)
        effects::KShadowHelper::self()->setWidget(this,d->m_radius);
}

bool KTranslucentFloor::shadow()
{
    Q_D(KTranslucentFloor);
    return d->m_shadowFlag;
}

void KTranslucentFloor::setEnableBlur(bool flag)
{
    Q_D(KTranslucentFloor);
    d->m_enableBlur = flag;
}

bool KTranslucentFloor::enableBlur()
{
    Q_D(KTranslucentFloor);
    return d->m_enableBlur;
}

void KTranslucentFloor::setOpacity(qreal opacity)
{
    Q_D(KTranslucentFloor);
    d->m_opacity = opacity;
}

qreal KTranslucentFloor::opacity()
{
    Q_D(KTranslucentFloor);
    return d->m_opacity;
}

void KTranslucentFloor::paintEvent(QPaintEvent *event)
{
    Q_D(KTranslucentFloor);

    QPainterPath path;
    QRect rect = this->rect();
    path.addRoundedRect(rect,d->m_radius,d->m_radius);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);

    if(d->m_enableBlur)
    {
        //开启毛玻璃时
        QRegion region(path.toFillPolygon().toPolygon());
        KWindowEffects::enableBlurBehind(this->winId(),true,region);
        this->setMask(region);
        painter.setOpacity(d->m_opacity);
    }
    else
    {
        //未开启毛玻璃时
        painter.setOpacity(1);
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(this->palette().color(this->backgroundRole()));
    painter.drawPath(path);

}

KTranslucentFloorPrivate::KTranslucentFloorPrivate(KTranslucentFloor *parent)
    :q_ptr(parent),
     m_radius(12),
     m_shadowFlag(true),
     m_enableBlur(true),
     m_opacity(0.5)
{
}
}

#include "ktranslucentfloor.moc"
#include "moc_ktranslucentfloor.cpp"
