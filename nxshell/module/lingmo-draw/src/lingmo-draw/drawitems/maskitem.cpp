// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maskitem.h"
#include "pagescene.h"
#include "pageitem_p.h"
#include <QPainter>
REGISTITEMCLASS(MaskItem, MaskItemType)

MaskItem::MaskItem(PageScene *scene):
    PageItem(nullptr)
{
    setZValue(10000);
    if (scene != nullptr) {
        setRect(scene->sceneRect());
    } else {
        setRect(QRectF(0, 0, 1920, 1080));
    }
}

int MaskItem::type() const
{
    return MaskItemType;
}

QRectF MaskItem::itemRect() const
{
    return rect();
}

void MaskItem::setRect(const QRectF &rect)
{
    prepareGeometryChange();
    m_rect = rect;
    updateShape();
}

QRectF MaskItem::rect() const
{
    return m_rect;
}

bool MaskItem::contains(const QPointF &point) const
{
    return false;
}

bool MaskItem::isPosPenetrable(const QPointF &posLocal)
{
    return true;
}

bool MaskItem::isRectPenetrable(const QRectF &rectLocal)
{
    return true;
}

void MaskItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setBrush(Qt::transparent);
    painter->setPen(Qt::NoPen);
    painter->drawRect(orgRect());
    PageItem::paint(painter, option, widget);

    if (pageScene()->page()->currentTool() == cut) {
        PageItem* cutItem = nullptr;
        QList<PageItem *> pageItems = pageScene()->allPageItems();
        foreach (auto item, pageItems) {
            if (item && item->type() == CutType) {
                cutItem = item;
                break;
            }
        }

        if (cutItem && cutItem->isVisible()) {
            painter->save();
            QRectF layerRct = mapToScene(itemRect()).boundingRect();
            QRectF cutRct = cutItem->mapToScene(cutItem->itemRect()).boundingRect();
            QRegion r1(cutRct.toRect());
            QRegion r2(layerRct.toRect());
            QRegion r3 = r2.subtracted(r1);
            QPainterPath path;
            path.addRegion(r3);

            QColor background(0, 0, 0, 51);
            painter->setPen(Qt::NoPen);
            painter->setBrush(background);
            painter->drawPath(path);
            painter->restore();
        }
    }
}

void MaskItem::updateShape()
{
//    QRectF rct;
//    if (scene() != nullptr) {
//        rct = scene()->sceneRect();
//    }
//    setRect(rct);
}
