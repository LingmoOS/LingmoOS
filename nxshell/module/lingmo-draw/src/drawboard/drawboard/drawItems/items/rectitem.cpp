// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "rectitem.h"
#include "csizehandlerect.h"
#include "pageview.h"
#include "cgraphicsitemevent.h"
#include "attributewidget.h"

#include <QPainter>
#include <QPixmap>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPainter>
#include <QDebug>
#include <QGraphicsBlurEffect>
#include <QtMath>

REGISTITEMCLASS(RectItem, RectType)
RectBaseItem::RectBaseItem(PageItem *parent)
    : RectBaseItem(0, 0, 0, 0, parent)
{
}

RectBaseItem::RectBaseItem(const QRectF &rect, PageItem *parent)
    : RectBaseItem(rect.left(), rect.top(), rect.width(), rect.height(), parent)
{

}

RectBaseItem::RectBaseItem(qreal x, qreal y, qreal w, qreal h, PageItem *parent)
    : VectorItem(parent), _rct(x, y, w, h)
{
}

RectBaseItem::~RectBaseItem()
{
}

QRectF RectBaseItem::rect() const
{
    return _rct;
}

void RectBaseItem::setRect(const QRectF &rect)
{
    if (rect != _rct) {
        preparePageItemGeometryChange();
        _rct = rect;
        updateShape();
    }
}

QPainterPath RectBaseItem::getOrgShape() const
{
    return calOrgShapeBaseRect(rect());
}

void RectBaseItem::loadUnit(const Unit &data)
{
    RectUnitData i = data.value<RectUnitData>();
    loadVectorData(data.head);
    loadGraphicsRectUnit(i);
}

Unit RectBaseItem::getUnit(int reson) const
{
    Unit unit;

    unit.usage = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(RectUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    RectUnitData i;
    i.topLeft = _rct.topLeft();
    i.bottomRight = _rct.bottomRight();
//    i.xRedius = m_xRedius;
//    i.yRedius = m_yRedius;
    unit.data.data.setValue<RectUnitData>(i);

    return unit;
}

//QRectF RectBaseItem::itemRect() const
//{
//    return VectorItem::itemRect();
//}

//QRectF RectBaseItem::orgRect() const
//{
//    return rect();
//}

void RectBaseItem::doScaling(PageItemScalEvent *event)
{
    QTransform trans = event->trans();
    QRectF rct = this->rect();
    QPointF pos1 = trans.map(rct.topLeft());
    QPointF pos4 = trans.map(rct.bottomRight());
    QRectF newRect = QRectF(pos1, pos4);

    if (newRect.isValid()) {
        this->setRect(newRect);
    }
}

bool RectBaseItem::testScaling(PageItemScalEvent *event)
{
    static QSizeF s_minSize = QSizeF(5, 5);
    bool accept = true;
    QTransform trans = event->trans();
    QRectF rct = this->orgRect();
    QPointF pos1 = trans.map(rct.topLeft());
    QPointF pos4 = trans.map(rct.bottomRight());
    QRectF newRect = QRectF(pos1, pos4);
    event->setMayResultPolygon(this->mapToScene(newRect));
    accept = newRect.isValid();
    if (accept) {
        if (newRect.width() < s_minSize.width() || newRect.height() < s_minSize.height())
            accept = false;
    }
//    if (accept) {
//        if (event->driverEvent() != nullptr) {
//            auto driverEvent = static_cast<CGraphItemScalEvent *>(event->driverEvent());
//            auto driverFatherScenePolygon = event->driverEvent()->item()->mapFromScene(driverEvent->mayResultPolygon());
//            auto selfWantedScenePolygon = event->driverEvent()->item()->mapFromScene(event->mayResultPolygon());
//            accept = driverFatherScenePolygon.boundingRect().adjusted(-1, -1, 1, 1).contains(selfWantedScenePolygon.boundingRect());
//        }
//    }
    return accept;
}

void RectItem::setXYRadius(int xRedius, int yRedius, bool preview)
{
    if (!preview) {
        m_xRadius = xRedius;
        m_yRadius = yRedius;
    } else {
        m_radiusForPreview = xRedius;
    }
    m_isPreviewRadius = preview;

    updateShape();
}

int RectItem::getXRadius() const
{
    return m_xRadius;
}

void RectItem::setRectRadius(QVariantList &radius, bool preview)
{
    if (radius.isEmpty()) {
        return;
    }
    if (1 == radius.size()) {
        m_bSameRadiusModel = true;
        setXYRadius(radius.at(0).toInt(), radius.at(0).toInt(), preview);
        m_leftRadius = m_xRadius;
        m_rightRadius = m_xRadius;
        m_leftBottomRadius = m_xRadius;
        m_rightBottomRadius = m_xRadius;
    } else {
        m_bSameRadiusModel = false;
        int i = 0;
        if (radius.at(i).isValid()) {
            m_leftRadius = radius.at(i).toInt();
        }

        if (radius.at(++i).isValid()) {
            m_rightRadius = radius.at(i).toInt();
        }

        if (radius.at(++i).isValid()) {
            m_leftBottomRadius = radius.at(i).toInt();
        }

        if (radius.at(++i).isValid()) {
            m_rightBottomRadius = radius.at(i).toInt();
        }

        m_isPreviewRadius = preview;
        updateShape();
    }
}

void RectItem::loadUnit(const Unit &ut)
{
    RectBaseItem::loadUnit(ut);
    RectUnitData d = ut.value<RectUnitData>();
    if (d.bSameRadius) {
        setRectRadius(QVariantList() << d.xRedius);
    } else {
        setRectRadius(QVariantList() << d.leftRadius << d.rightRadius << d.leftBottomRadius << d.rightBottomRadius);
    }

    m_isPreviewRadius = false;
}

Unit RectItem::getUnit(int reson) const
{
    Unit ut = RectBaseItem::getUnit(reson);
    RectUnitData d = ut.value<RectUnitData>();
    d.xRedius = m_xRadius;
    d.yRedius = m_yRadius;
    d.bSameRadius = m_bSameRadiusModel;
    d.leftRadius = m_leftRadius;
    d.rightRadius = m_rightRadius;
    d.leftBottomRadius = m_leftBottomRadius;
    d.rightBottomRadius = m_rightBottomRadius;
    ut.setValue<RectUnitData>(d);
    return ut;
}

void RectBaseItem::loadGraphicsRectUnit(const RectUnitData &rectData)
{
    setRect(QRectF(rectData.topLeft, rectData.bottomRight));
}

int RectItem::type() const
{
    return RectType;
}

SAttrisList RectItem::attributions()
{
    SAttrisList result;
    QVariantList radius;
    if (m_bSameRadiusModel) {
        radius << m_leftRadius;
    } else {
        radius << m_leftRadius << m_rightRadius << m_leftBottomRadius << m_rightBottomRadius;
    }

    result << SAttri(ERectRadius,  radius);
    return result.unionAtrri(VectorItem::attributions());
}

void RectItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    if (attri ==  ERectRadius) {
        bool isPreview = (phase == EChangedBegin || phase == EChangedUpdate);
        auto l = var.toList();
        setRectRadius(l, isPreview);
        //setXYRadius(var.toInt(), var.toInt(), isPreview);
        return;
    }
    VectorItem::setAttributionVar(attri, var, phase);
}

QPainterPath RectItem::calOrgShapeBaseRect(const QRectF &rect) const
{
    QPainterPath path;
    if (m_bSameRadiusModel) {
        path.addRoundedRect(rect, m_xRadius, m_yRadius, Qt::AbsoluteSize);;
    } else {
        qreal leftRadius = m_leftRadius;
        qreal rightRadius = m_rightRadius;
        qreal leftBottomRadius = m_leftBottomRadius;
        qreal rightBottomRadius = m_rightBottomRadius;

        calibrationRadius(leftRadius, rightRadius, leftBottomRadius, rightBottomRadius, rect);

        if (0 >= leftRadius) {
            path.moveTo(rect.topLeft());
        } else {
            path.arcMoveTo(rect.topLeft().x(), rect.topLeft().y(), 2 * leftRadius, 2 * leftRadius, 180);
            path.arcTo(rect.topLeft().x(), rect.topLeft().y(), 2 * leftRadius, 2 * leftRadius, 180, -90);
        }

        if (0 >= rightRadius) {
            path.lineTo(rect.topRight());
        } else {
            path.arcTo(rect.topRight().x() - 2 * rightRadius, rect.topRight().y(), 2 * rightRadius, 2 * rightRadius, 90, -90);
        }

        if (0 >= rightBottomRadius) {
            path.lineTo(rect.bottomRight());
        } else {
            path.arcTo(rect.bottomRight().x() - 2 * rightBottomRadius, rect.bottomRight().y() - 2 * rightBottomRadius, 2 * rightBottomRadius, 2 * rightBottomRadius, 0, -90);
        }

        if (0 >= leftBottomRadius) {
            path.lineTo(rect.bottomLeft());
        } else {
            path.arcTo(rect.bottomLeft().x(), rect.bottomLeft().y() - 2 * leftBottomRadius, 2 * leftBottomRadius, 2 * leftBottomRadius, 270, -90);
        }


        path.closeSubpath();
    }
    return path;
}

void RectItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    if (type() == RectType && m_isPreviewRadius) {
        beginCheckIns(painter);
        const QPen curPen = this->paintPen();
        painter->setPen(curPen.width() == 0 || !isDarwBorder() ? Qt::NoPen : curPen);
        painter->setBrush(isDrawFill() ? this->paintBrush() : Qt::NoBrush);
        painter->drawRoundedRect(this->orgRect(), m_radiusForPreview, m_radiusForPreview, Qt::AbsoluteSize);
        endCheckIns(painter);

        paintMutBoundingLine(painter, option);
    } else {
        VectorItem::paintSelf(painter, option);
    }
}

void RectItem::calibrationRadius(qreal &left, qreal &right, qreal &leftBottom, qreal &rightBottom, const QRectF &rect) const
{
    qreal width = rect.width();
    qreal height = rect.height();

    left = qMin(width, left);
    left = qMin(height, left);

    right = qMin(width - left, right);
    right = qMin(height, right);

    rightBottom = qMin(height - right, rightBottom);
    rightBottom = qMin(width, rightBottom);

    leftBottom = qMin(height - left, leftBottom);
    leftBottom = qMin(width - rightBottom, leftBottom);

    qreal defaultValue = 0;
    left = left > 0 ? left : defaultValue ;
    right = right > 0 ? right : defaultValue;
    leftBottom = leftBottom > 0 ? leftBottom : defaultValue;
    rightBottom = rightBottom > 0 ? rightBottom : defaultValue;
}
