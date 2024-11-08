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

#include "kshadowhelper.h"
#include <QPixmap>
#include <QLinearGradient>
#include <QPalette>
#include <KWindowShadow>
#include <KWindowShadowTile>
#include <QPainter>
#include <QMap>

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

namespace kdk {
namespace effects {

class KShadowHelperPrivate:public QObject
{
    Q_DECLARE_PUBLIC(KShadowHelper)
    Q_OBJECT

public:
    KShadowHelperPrivate(KShadowHelper*parent);
    QPixmap getShadowPixmap(QColor color,
                            int shadowBorder,
                            qreal darkness,
                            int borderRadius = 0);
    KWindowShadow *getShadow(QColor color, int shadowBorder,
                             qreal darkness,
                             int borderRadius = 0);
protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    KShadowHelper* q_ptr;
    QMap<const QWidget *, KWindowShadow *> m_shadows;
};

static KShadowHelper* g_kshadowHelper = nullptr;

KShadowHelper *KShadowHelper::self()
{
    if(g_kshadowHelper)
        return g_kshadowHelper;
    g_kshadowHelper = new KShadowHelper();
    return g_kshadowHelper;
}

void KShadowHelper::setWidget(QWidget *widget, int borderRadius, int shadowWidth, qreal darkness)
{
    Q_D(KShadowHelper);
    KWindowShadow* shadow = d->getShadow(QColor(26,26,26),shadowWidth,darkness,borderRadius);
    shadow->setPadding(QMargins(shadowWidth,shadowWidth,shadowWidth,shadowWidth));

    widget->installEventFilter(d);
    d->m_shadows.insert(widget, shadow);
    connect(widget, &QWidget::destroyed, this, [=](){
        if (auto shadowToBeDelete = d->m_shadows.value(widget)) {
            if (shadowToBeDelete->isCreated())
                shadowToBeDelete->destroy();
            shadowToBeDelete->deleteLater();
            d->m_shadows.remove(widget);
        }
    });
}

KShadowHelper::KShadowHelper(QObject *parent)
    : QObject(parent),
      d_ptr(new KShadowHelperPrivate(this))
{

}

KShadowHelperPrivate::KShadowHelperPrivate(KShadowHelper *parent)
    :q_ptr(parent)
{

}

QPixmap KShadowHelperPrivate::getShadowPixmap(QColor color, int shadowBorder, qreal darkness, int borderRadius)
{
    QPixmap pixmap(QSize(100,100));
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(pixmap.rect().adjusted(shadowBorder,shadowBorder,-shadowBorder,-shadowBorder),borderRadius,borderRadius);

    QImage rawImg = pixmap.toImage();
    qt_blurImage(rawImg, shadowBorder, true, true);

    for (int x = 0; x < rawImg.width(); x++) {
        for (int y = 0; y < rawImg.height(); y++) {
            auto color = rawImg.pixelColor(x, y);
            if (color.alpha() == 0)
                continue;
            color.setAlphaF(darkness * color.alphaF());
            rawImg.setPixelColor(x, y, color);
        }
    }
    QPixmap target = QPixmap::fromImage(rawImg);
    QPainter painter2(&target);
    painter2.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter2.setCompositionMode(QPainter::CompositionMode_Clear);
    painter2.setBrush(Qt::transparent);
    painter2.setPen(Qt::NoPen);
    painter2.drawRoundedRect(target.rect().adjusted(shadowBorder,shadowBorder,-shadowBorder,-shadowBorder),borderRadius,borderRadius);
    return target;
}

KWindowShadow *KShadowHelperPrivate::getShadow(QColor color, int shadowBorder, qreal darkness, int borderRadius)
{
    QPixmap pixmap = getShadowPixmap(color,shadowBorder,darkness,borderRadius);
    int parm = shadowBorder*2;
    QPixmap topLeftPixmap = pixmap.copy(0, 0, parm, parm);
    QPixmap topPixmap = pixmap.copy(parm, 0, pixmap.width()-parm*2, parm);
    QPixmap topRightPixmap = pixmap.copy(pixmap.width()-parm, 0, parm, parm);
    QPixmap leftPixmap = pixmap.copy(0, parm, parm, pixmap.height()-parm*2);
    QPixmap rightPixmap = pixmap.copy(pixmap.width()-parm, parm, parm, pixmap.height()-parm*2);
    QPixmap bottomLeftPixmap = pixmap.copy(0, pixmap.height()-parm, parm, parm);
    QPixmap bottomPixmap = pixmap.copy(parm, pixmap.height()-parm, pixmap.width()-parm*2, parm);
    QPixmap bottomRightPixmap = pixmap.copy(pixmap.width()-parm,pixmap.height()-parm, parm, parm);

    KWindowShadow *shadow = new KWindowShadow;
    KWindowShadowTile::Ptr topLeftTile = KWindowShadowTile::Ptr::create();
    topLeftTile.get()->setImage(topLeftPixmap.toImage());
    shadow->setTopLeftTile(topLeftTile);
    KWindowShadowTile::Ptr topTile = KWindowShadowTile::Ptr::create();
    topTile.get()->setImage(topPixmap.toImage());
    shadow->setTopTile(topTile);
    KWindowShadowTile::Ptr topRightTile = KWindowShadowTile::Ptr::create();
    topRightTile.get()->setImage(topRightPixmap.toImage());
    shadow->setTopRightTile(topRightTile);
    KWindowShadowTile::Ptr leftTile = KWindowShadowTile::Ptr::create();
    leftTile.get()->setImage(leftPixmap.toImage());
    shadow->setLeftTile(leftTile);
    KWindowShadowTile::Ptr rightTile = KWindowShadowTile::Ptr::create();
    rightTile.get()->setImage(rightPixmap.toImage());
    shadow->setRightTile(rightTile);
    KWindowShadowTile::Ptr bottomLeftTile = KWindowShadowTile::Ptr::create();
    bottomLeftTile.get()->setImage(bottomLeftPixmap.toImage());
    shadow->setBottomLeftTile(bottomLeftTile);
    KWindowShadowTile::Ptr bottomTile = KWindowShadowTile::Ptr::create();
    bottomTile.get()->setImage(bottomPixmap.toImage());
    shadow->setBottomTile(bottomTile);
    KWindowShadowTile::Ptr bottomRightTile = KWindowShadowTile::Ptr::create();
    bottomRightTile.get()->setImage(bottomRightPixmap.toImage());
    shadow->setBottomRightTile(bottomRightTile);
    return shadow;
}

bool KShadowHelperPrivate::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->isWidgetType())
    {
        auto widget = qobject_cast<QWidget *>(watched);
        if(widget->isTopLevel() && event->type() == QEvent::Show)
        {
            if (auto shadow = m_shadows.value(widget))
            {
                shadow->setWindow(widget->windowHandle());
                shadow->create();
            }

        }
    }
    return QObject::eventFilter(watched,event);
}

}
}
#include "kshadowhelper.moc"
#include "moc_kshadowhelper.cpp"
