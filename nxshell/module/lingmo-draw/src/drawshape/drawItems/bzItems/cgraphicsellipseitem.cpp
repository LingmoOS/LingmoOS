// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicsellipseitem.h"
#include <QPainter>
#include <QPainterPath>
//#include "cgraphicsmasicoitem.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "../../../service/dyncreatobject.h"

REGISTITEMCLASS(CGraphicsEllipseItem, EllipseType)
CGraphicsEllipseItem::CGraphicsEllipseItem(CGraphicsItem *parent)
    : CGraphicsRectItem(parent)
{

}

CGraphicsEllipseItem::CGraphicsEllipseItem(qreal x, qreal y, qreal w, qreal h, CGraphicsItem *parent)
    : CGraphicsRectItem(x, y, w, h, parent)
{

}

QPainterPath CGraphicsEllipseItem::getSelfOrgShape() const
{
    QPainterPath path;
    path.addEllipse(rect()); //添加矩形的内椭圆
    path.closeSubpath();
    return path;
}

int CGraphicsEllipseItem::type() const
{
    return EllipseType;
}

DrawAttribution::SAttrisList CGraphicsEllipseItem::attributions()
{
    return CGraphicsItem::attributions();
}

void CGraphicsEllipseItem::loadGraphicsUnit(const CGraphicsUnit &data)
{
    if (data.data.pCircle != nullptr)
        loadGraphicsRectUnit(data.data.pCircle->rect);

    loadHeadData(data.head);

    updateShape();
}

CGraphicsUnit CGraphicsEllipseItem::getGraphicsUnit(EDataReason reson) const
{
    CGraphicsUnit unit;

    unit.reson = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(SGraphicsCircleUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    unit.data.pCircle = new SGraphicsCircleUnitData();
    unit.data.pCircle->rect.topLeft = this->rect().topLeft();
    unit.data.pCircle->rect.bottomRight = this->rect().bottomRight();

    return unit;
}
