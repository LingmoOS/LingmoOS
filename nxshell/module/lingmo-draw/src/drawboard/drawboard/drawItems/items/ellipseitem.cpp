// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ellipseitem.h"
#include "pageview.h"

#include <QPainter>
#include <QPainterPath>

REGISTITEMCLASS(EllipseItem, EllipseType)
EllipseItem::EllipseItem(PageItem *parent)
    : RectBaseItem(parent)
{

}

EllipseItem::EllipseItem(qreal x, qreal y, qreal w, qreal h, PageItem *parent)
    : RectBaseItem(x, y, w, h, parent)
{

}

QPainterPath EllipseItem::calOrgShapeBaseRect(const QRectF &rect) const
{
    QPainterPath path;
    path.addEllipse(rect); //添加矩形的内椭圆
    path.closeSubpath();
    return path;
}

int EllipseItem::type() const
{
    return EllipseType;
}

SAttrisList EllipseItem::attributions()
{
    return VectorItem::attributions();
}

void EllipseItem::loadUnit(const Unit &data)
{
//    if (data.data.pCircle != nullptr)
//        loadGraphicsRectUnit(data.data.pCircle->rect);
    EllipseUnitData i = data.data.data.value<EllipseUnitData>();
    loadVectorData(data.head);
    loadGraphicsRectUnit(i.rect);



    //updateShape();
}

Unit EllipseItem::getUnit(int reson) const
{
    Unit unit;

    unit.usage = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(EllipseUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

//    unit.data.pCircle = new SGraphicsCircleUnitData();
//    unit.data.pCircle->rect.topLeft = this->rect().topLeft();
//    unit.data.pCircle->rect.bottomRight = this->rect().bottomRight();
    EllipseUnitData itemData;
    itemData.rect.topLeft = this->orgRect().topLeft();
    itemData.rect.bottomRight = this->orgRect().bottomRight();
    unit.data.data.setValue<EllipseUnitData>(itemData);

    return unit;
}
