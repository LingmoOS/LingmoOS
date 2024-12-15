// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicstriangleitem.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cattributeitemwidget.h"
#include "../../../service/dyncreatobject.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QtMath>

REGISTITEMCLASS(CGraphicsTriangleItem, TriangleType)
CGraphicsTriangleItem::CGraphicsTriangleItem(CGraphicsItem *parent)
    : CGraphicsRectItem(parent)
{

}

CGraphicsTriangleItem::CGraphicsTriangleItem(const QRectF &rect, CGraphicsItem *parent)
    : CGraphicsRectItem(rect, parent)
{

}

CGraphicsTriangleItem::CGraphicsTriangleItem(qreal x, qreal y, qreal w, qreal h, CGraphicsItem *parent)
    : CGraphicsRectItem(x, y, w, h, parent)
{

}

DrawAttribution::SAttrisList CGraphicsTriangleItem::attributions()
{
    return CGraphicsItem::attributions();
}

void CGraphicsTriangleItem::loadGraphicsUnit(const CGraphicsUnit &data)
{
    if (data.data.pTriangle != nullptr) {
        loadGraphicsRectUnit(data.data.pTriangle->rect);
    }
    loadHeadData(data.head);
    updateShape();
}

CGraphicsUnit CGraphicsTriangleItem::getGraphicsUnit(EDataReason reson) const
{
    Q_UNUSED(reson)

    CGraphicsUnit unit;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(SGraphicsTriangleUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    unit.data.pTriangle = new SGraphicsTriangleUnitData();
    unit.data.pTriangle->rect.topLeft = this->rect().topLeft();
    unit.data.pTriangle->rect.bottomRight = this->rect().bottomRight();

    return  unit;
}

QPainterPath CGraphicsTriangleItem::getSelfOrgShape() const
{
    QPolygonF ply;
    calcPoints_helper(ply);

    QPainterPath path;
    path.addPolygon(ply);
    path.closeSubpath();
    return path;
}

int CGraphicsTriangleItem::type() const
{
    return TriangleType;
}

void CGraphicsTriangleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    beginCheckIns(painter);
    painter->save();

    painter->setPen(paintPen().width() == 0 ? Qt::NoPen : paintPen());
    painter->setBrush(paintBrush());
    painter->drawPath(selfOrgShape());

    painter->restore();
    endCheckIns(painter);

    paintMutBoundingLine(painter, option);
}

void CGraphicsTriangleItem::calcPoints_helper(QVector<QPointF> &outVector, qreal offset) const
{
    QRectF rc = rect();
    QPointF top = QPointF((rc.x() + rc.width() / 2), rc.y());
    const QPen pen = this->paintPen();

    outVector.clear();
    //获取填充区域路径
    qreal offsetWidth = offset;
    QLineF line1(top, rc.bottomLeft());
    QLineF line2(rc.bottomLeft(), rc.bottomRight());
    QLineF line3(rc.bottomRight(), top);
    QVector<QLineF> lines;
    lines << line3 << line1 << line2;
    for (int i = 0; i < lines.size(); ++i) {
        QLineF ln1  = lines.at(i);
        QLineF ln2  = (i == lines.size() - 1 ? lines[0] : lines[i + 1]);
        qreal angle = 180 - ln1.angleTo(ln2);

        qreal offsetLen = offsetWidth / qSin(qDegreesToRadians(angle / 2.0));
        QLineF tempLine(ln2);
        tempLine.setAngle(tempLine.angle() + angle / 2.0);
        tempLine.setLength(offsetLen);

        outVector.append(tempLine.p2());
    }
}
