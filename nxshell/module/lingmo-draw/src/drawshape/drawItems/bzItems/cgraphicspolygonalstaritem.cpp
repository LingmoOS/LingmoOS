// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicspolygonalstaritem.h"
#include "cgraphicspolygonitem.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cgraphicsitemevent.h"
#include "cattributeitemwidget.h"
#include "cpolygonalstartool.h"
#include "../../../service/dyncreatobject.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

REGISTITEMCLASS(CGraphicsPolygonalStarItem, PolygonalStarType)
CGraphicsPolygonalStarItem::CGraphicsPolygonalStarItem(int anchorNum, int innerRadius, CGraphicsItem *parent)
    : CGraphicsRectItem(parent)
{
    updatePolygonalStar(anchorNum, innerRadius);
}

CGraphicsPolygonalStarItem::CGraphicsPolygonalStarItem(int anchorNum, int innerRadius, const QRectF &rect, CGraphicsItem *parent)
    : CGraphicsRectItem(rect, parent)
{
    updatePolygonalStar(anchorNum, innerRadius);
}

CGraphicsPolygonalStarItem::CGraphicsPolygonalStarItem(int anchorNum, int innerRadius, qreal x, qreal y, qreal w, qreal h, CGraphicsItem *parent)
    : CGraphicsRectItem(x, y, w, h, parent)
{
    updatePolygonalStar(anchorNum, innerRadius);
}

DrawAttribution::SAttrisList CGraphicsPolygonalStarItem::attributions()
{
    DrawAttribution::SAttrisList result;
    result << DrawAttribution::SAttri(DrawAttribution::EBrushColor, brush().color())
           << DrawAttribution::SAttri(DrawAttribution::EPenColor, pen().color())
           << DrawAttribution::SAttri(DrawAttribution::EBorderWidth,  pen().width())
           << DrawAttribution::SAttri(CPolygonalStarTool::EStartLineSep)
           << DrawAttribution::SAttri(DrawAttribution::EStarAnchor,  anchorNum())
           << DrawAttribution::SAttri(DrawAttribution::EStarInnerOuterRadio,  innerRadius());
    return result;
}

void CGraphicsPolygonalStarItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    bool isPreview = (phase == EChangedBegin || phase == EChangedUpdate);
    if (DrawAttribution::EStarAnchor == attri) {
        setAnchorNum(var.toInt(), isPreview);
        updateShapeRecursion();
        updateViewport();
    } else if (DrawAttribution::EStarInnerOuterRadio == attri) {
        setInnerRadius(var.toInt(), isPreview);
        updateViewport();
    } else {
        CGraphicsItem::setAttributionVar(attri, var, phase);
    }
}

int CGraphicsPolygonalStarItem::type() const
{
    return PolygonalStarType;
}

CGraphicsUnit CGraphicsPolygonalStarItem::getGraphicsUnit(EDataReason reson) const
{
    CGraphicsUnit unit;

    unit.reson = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(SGraphicsPolygonStarUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    unit.data.pPolygonStar = new SGraphicsPolygonStarUnitData();
    unit.data.pPolygonStar->rect.topLeft = this->rect().topLeft();
    unit.data.pPolygonStar->rect.bottomRight = this->rect().bottomRight();
    unit.data.pPolygonStar->anchorNum = this->m_anchorNum[0];
    unit.data.pPolygonStar->radius = this->m_innerRadius[0];

    return  unit;
}

void CGraphicsPolygonalStarItem::updateShape()
{
    //CGraphicsRectItem::updateShape();
    m_selfOrgPathShape   = getSelfOrgShape();
    m_penStroerPathShape = getPenStrokerShape();
    m_boundingShape      = getShape();

    m_boundingRect       = getGraphicsItemShapePathByOrg(selfOrgShape(), pen(), true, this->incLength(), false).controlPointRect();

    m_boundingShapeTrue  = getTrulyShape();
    m_boundingRectTrue   = m_boundingShapeTrue.controlPointRect();

//    if (drawScene() != nullptr)
//        drawScene()->selectGroup()->updateBoundingRect();
}

QPainterPath CGraphicsPolygonalStarItem::getSelfOrgShape() const
{
    QPolygonF ply;
    calcPolygon_helper(ply, anchorNum());

    QPainterPath path;
    path.addPolygon(ply);
    path.closeSubpath();
    return path;
}

QPainterPath CGraphicsPolygonalStarItem::getPenStrokerShape() const
{
    QPainterPath path;

    QPolygonF plyOuter;
    calcPolygon_helper(plyOuter, anchorNum(), paintPen().widthF() / 2);
    path.addPolygon(plyOuter);

    QPolygonF plyInner;
    calcPolygon_helper(plyInner, anchorNum(), -paintPen().widthF() / 2);
    path.addPolygon(plyInner);

    return path;
}

QPainterPath CGraphicsPolygonalStarItem::getShape() const
{
    QPolygonF ply;
    calcPolygon_helper(ply, anchorNum(), paintPen().widthF() / 2 + this->incLength());

    QPainterPath path;
    path.addPolygon(ply);
    path.closeSubpath();
    return path;
}

QPainterPath CGraphicsPolygonalStarItem::getTrulyShape() const
{
    QPainterPath path;

    QPolygonF plyOuter;
    calcPolygon_helper(plyOuter, anchorNum(), paintPen().widthF() / 2);
    path.addPolygon(plyOuter);
    return path;
}

void CGraphicsPolygonalStarItem::setRect(const QRectF &rect)
{
    CGraphicsRectItem::setRect(rect);
}

void CGraphicsPolygonalStarItem::updatePolygonalStar(int anchorNum, int innerRadius)
{
    m_anchorNum[0] = anchorNum;
    m_innerRadius[0] = innerRadius;
    m_preview[0] = false;
    m_preview[1] = false;

    updateShape();

    if (isCached())
        resetCachePixmap();
}

void CGraphicsPolygonalStarItem::setAnchorNum(int num, bool preview)
{
    m_preview[0] = preview;
    m_anchorNum[preview] = num;
    updateShape();
    if (isCached())
        resetCachePixmap();
}

void CGraphicsPolygonalStarItem::loadGraphicsUnit(const CGraphicsUnit &data)
{
    if (data.data.pPolygonStar != nullptr) {
        loadGraphicsRectUnit(data.data.pPolygonStar->rect);
        updatePolygonalStar(data.data.pPolygonStar->anchorNum, data.data.pPolygonStar->radius);
    }

    loadHeadData(data.head);
}

int CGraphicsPolygonalStarItem::innerRadius() const
{
    return m_innerRadius[m_preview[1]];
}

int CGraphicsPolygonalStarItem::anchorNum() const
{
    return m_anchorNum[m_preview[0]];
}

void CGraphicsPolygonalStarItem::setInnerRadius(int radius, bool preview)
{
    bool changed = (m_preview[1] != preview || m_innerRadius[preview] != radius);
    m_preview[1] = preview;
    m_innerRadius[preview] = radius;

    if (changed)
        updateShapeRecursion();

    if (isCached())
        resetCachePixmap();
}

void CGraphicsPolygonalStarItem::calcPolygon_helper(QPolygonF &outPolygon, int n, qreal offset) const
{
    if (n == 0)return;

    outPolygon.clear();

//    //根据垂直于边线的方向偏移offset得到外圆半径的偏移值finalOffset
    qreal angle = qDegreesToRadians(90.);
    QPointF pointCenter = this->rect().center();

    qreal outerEllipseXDiameter = this->rect().width();
    qreal outerEllipseYDiameter = this->rect().height();

    qreal outer_w = outerEllipseXDiameter / 2.0;
    qreal outer_h = outerEllipseYDiameter / 2.0;

    //根据外圆和内圆的关系计算内圆的半径
    qreal inner_w = outer_w * innerRadius() / 100.0;
    qreal inner_h = outer_h * innerRadius() / 100.0;

    if (n > 0) {
        qreal preAngle = qDegreesToRadians(360. / n);
        qreal innerAngle = angle + preAngle / 2;
        for (int i = 0; i < n; i++) {

            qreal outer_Angle = angle + preAngle * i;
            qreal inner_Angle = innerAngle + preAngle * i;

            qreal outer_x = pointCenter.x() + outer_w * qCos(outer_Angle);
            qreal outer_y = pointCenter.y() - outer_h  * qSin(outer_Angle);


            //偶数是外圈点
            outPolygon.push_back(QPointF(outer_x, outer_y));

            qreal inner_x = pointCenter.x() + inner_w * qCos(inner_Angle);
            qreal inner_y = pointCenter.y() - inner_h  * qSin(inner_Angle);

            //奇数是内圈点
            outPolygon.push_back(QPointF(inner_x, inner_y));

        }
    }

    if (!qFuzzyIsNull(offset)) {
        QList<QLineF> outlines;
        auto fGetLines = [ = ](const QPolygonF & outPolygon, QList<QLineF> &resultLines) {
            resultLines.clear();
            for (int i = 0; i < outPolygon.size(); ++i) {
                if (i != 0) {
                    resultLines.append(QLineF(outPolygon.at(i - 1), outPolygon.at(i)));
                }
            }
            resultLines.push_front(QLineF(outPolygon.last(), outPolygon.first()));
        };

        fGetLines(outPolygon, outlines);

        auto fGetOffsetPos = [ = ](const QList<QLineF> &lines, QVector<QPointF> &result) {
            result.clear();
            for (int i = 0; i < lines.size(); ++i) {
                QLineF curLine  = lines[i];
                QLineF nextLine = (i == lines.size() - 1 ? lines[0] : lines[i + 1]);

                bool isInter = /*(i%2!=0)*/curLine.angleTo(nextLine) > 180;   //是否这个线条是和内圈点相关
                qreal inc    = (isInter ? -1 : 1); //内圈点是相反的角度

                qreal finalDegree = (180 - curLine.angleTo(nextLine)) * inc; //两条线相交的交角*/

                qreal sinValue = qSin(qDegreesToRadians(finalDegree / 2.));

                qreal offLen = qFuzzyIsNull(sinValue) ? 0.0 : offset / sinValue;

                QLineF tempLine(nextLine);
                qreal newAngle = tempLine.angle() + finalDegree / 2.0 + (isInter ? (360 - curLine.angleTo(nextLine)) : 0) + (offset > 0 ? 180 : 0);
                tempLine.setAngle(newAngle);
                qreal finallenth =  qAbs(offLen);

                if (finallenth > nextLine.length()) {
                    finallenth = nextLine.length();
                }
                tempLine.setLength(finallenth);
                result.append(tempLine.p2());
            }
        };

        QVector<QPointF> outecliPos;

        fGetOffsetPos(outlines, outecliPos);

        outPolygon.clear();
        for (int i = 0; i < outecliPos.size(); ++i) {
            outPolygon.append(outecliPos[i]);
        }
    }
}
