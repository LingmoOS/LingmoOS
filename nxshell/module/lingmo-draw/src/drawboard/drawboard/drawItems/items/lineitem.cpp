// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lineitem.h"
#include "pageview.h"
#include "cgraphicsitemevent.h"
#include "itemgroup.h"
#include "attributewidget.h"

#include <QSvgRenderer>

#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QtMath>
#include <QDebug>

//DTK_USE_NAMESPACE

REGISTITEMCLASS(LineItem, LineType)

class LineItem::LineItem_private
{
public:
    explicit LineItem_private(LineItem *qq): q(qq) {}

    void init()
    {
        q->addHandleNode(new LineHandleNode(LineHandleNode::LinePos1, q));
        q->addHandleNode(new LineHandleNode(LineHandleNode::LinePos2, q));
    }

    QLineF outOperatePos2Line(const QLineF &line, bool firstPos)
    {
        if (!line.isNull()) {
            //转向然后对p2进行操作
            return firstPos ? QLineF(line.p2(), line.p1()) : line;
        }
        return line;
    }
    QPointF linePos2AfterIncreaseLenth(const QLineF &line, qreal increasLenth)
    {
        QLineF tempLine = line;
        tempLine.setLength(line.length() + increasLenth);
        return tempLine.p2();
    }

    QPainterPath calEmptyRingStyle(QLineF &line, bool firstPos, int radius)
    {
        QPainterPath path;
        if (!line.isNull()) {
            //保证对p2进行操作
            QLineF  opLine = outOperatePos2Line(line, firstPos);
            QTransform transToHorStander;
            transToHorStander.translate(opLine.p1().x(), opLine.p1().y());
            transToHorStander.rotate(opLine.angle() - 360.);
            transToHorStander.translate(-opLine.p1().x(), -opLine.p1().y());
            QLineF tempHorLine = transToHorStander.map(opLine);
            QPointF center = linePos2AfterIncreaseLenth(tempHorLine, radius);
            QRectF boundingRect = QRectF(center - QPointF(radius, radius), QSizeF(2 * radius, 2 * radius));
            path.moveTo(center + QPointF(radius, 0));
            path.arcTo(boundingRect, 0, 360);
            //path.addEllipse(boundingRect);
            path = transToHorStander.inverted().map(path);
        }
        return path;
    }

    QPainterPath calSolidRingStyle(QLineF &line, bool firstPos, qreal radius)
    {
        auto path = calEmptyRingStyle(line, firstPos, radius);
        return path;
    }

    QPainterPath calNormalArrowStyle(QLineF &line, bool firstPos, int agree = 30, int lenth = 30)
    {
        QPainterPath path;
        if (!line.isNull()) {
            QLineF  opLine = outOperatePos2Line(line, firstPos);

            QLineF l1(opLine.p2(), opLine.p1());
            l1.setAngle(l1.angle() + agree);
            l1.setLength(lenth);

            QLineF l2(opLine.p2(), opLine.p1());
            l2.setAngle(l2.angle() - agree);
            l2.setLength(lenth);

            path.moveTo(l1.p2());
            path.lineTo(opLine.p2());
            path.lineTo(l2.p2());
            //path.closeSubpath();
        }
        return path;
    }

    QPainterPath calSolidArrowStyle(QLineF &line, bool firstPos)
    {
        QPainterPath path = calNormalArrowStyle(line, firstPos, true);
        //path.closeSubpath();
        return path;
    }

    QPainterPath calStyle(bool beginStyle)
    {
        QPainterPath path;
        QLineF line = q->line();
        switch (beginStyle ? m_startType : m_endType) {
        case noneLine:
            path = QPainterPath();
            break;
        case normalRing:
            path = calEmptyRingStyle(line, beginStyle, q->pen().widthF());
            break;
        case soildRing:
            path = calSolidRingStyle(line, beginStyle, q->pen().widthF());
            break;
        case normalArrow:
            path = calNormalArrowStyle(line, beginStyle);
            break;
        case soildArrow:
            path = calSolidArrowStyle(line, beginStyle);
            break;
        }
        return path;
    }

    inline bool beginStyleShouldSetBrush()
    {
        return m_startType == soildRing || m_startType == soildArrow;
    }
    inline bool endStyleShouldSetBrush()
    {
        return m_endType == soildRing || m_endType == soildArrow;
    }

    void calShowPath(QPainterPath &path, QLineF &outLine,
                     QPainterPath &beginStylePath, QPainterPath &endStylePath)
    {
        QLineF line = q->line();
        const qreal degrees = 30.;
        const qreal arrowLenth = 10 + q->pen().width() * 3;
        qreal radius = q->pen().widthF() * 2;
        switch (m_startType) {
        case noneLine:
            break;
        case normalRing:
        case soildRing: {
            QLineF tempLine(line.p2(), line.p1());
            tempLine.setLength(tempLine.length() - 2 * radius);
            beginStylePath = calEmptyRingStyle(tempLine, false, radius);
            path.addPath(beginStylePath);
            line.setP1(tempLine.p2());
            break;
        }
        case normalArrow:
        case soildArrow: {
            beginStylePath = calNormalArrowStyle(line, true, degrees, arrowLenth);
            if (m_startType == soildArrow) {
                QLineF tempLine(line.p2(), line.p1());
                tempLine.setLength(tempLine.length() - arrowLenth * qCos(qDegreesToRadians(degrees)));
                beginStylePath.closeSubpath();
                line.setP1(tempLine.p2());
            }
            path.addPath(beginStylePath);
            //line.setP1(tempLine.p2());
            break;
        }
        }

        switch (m_endType) {
        case noneLine:
            break;
        case normalRing:
        case soildRing: {

            QLineF tempLine(line);
            tempLine.setLength(tempLine.length() - 2 * radius);
            endStylePath = calEmptyRingStyle(tempLine, false, radius);
            path.addPath(endStylePath);
            line.setP2(tempLine.p2());
            break;
        }
        case normalArrow:
        case soildArrow: {
            endStylePath = calNormalArrowStyle(line, false, degrees, arrowLenth);
            if (m_endType == soildArrow) {
                QLineF tempLine(line);
                tempLine.setLength(tempLine.length() - arrowLenth * qCos(qDegreesToRadians(degrees)));
                endStylePath.closeSubpath();
                line.setP2(tempLine.p2());
            }
            path.addPath(endStylePath);

            break;
        }
        }
        path.moveTo(line.p1());
        path.lineTo(line.p2());

        outLine = line;
    }
    LineItem *q;
    ELinePosStyle m_startType;     // 起始点样式
    ELinePosStyle m_endType;       // 终点样式
    QLine showLine;
};

LineItem::LineItem(PageItem *parent)
    : LineItem(QLine(), parent)
{
}

LineItem::LineItem(const QPointF &p1, const QPointF &p2, PageItem *parent)
    : LineItem(QLineF(p1, p2), parent)
{
}

LineItem::LineItem(const QLineF &line, PageItem *parent)
    : LineBaseItem(line, parent), LineItem_d(new LineItem_private(this))
{
    d_LineItem()->init();
    setSingleSelectionBorderVisible(false);
}

LineItem::~LineItem()
{
}

SAttrisList LineItem::attributions()
{
    SAttrisList result;
    result <<  SAttri(EPenColor, pen().color())
           //<<  SAttri(EBorderWidth,  pen().width())
           <<  SAttri(EPenWidth,  pen().width())
           <<  SAttri(1775)
           << SAttri(EEnablePenStyle,  VectorItem::isDarwBorder())
           << SAttri(EStreakStyle)
           <<  SAttri(EStreakBeginStyle, pos1Style())
           <<  SAttri(EStreakEndStyle,  pos2Style());

    return result/*.insected(LineBaseItem::attributions())*/;
}

void LineItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    bool isPreview = (phase == EChangedBegin || phase == EChangedUpdate);
    switch (attri) {
    case  EPenColor: {
        setPenColor(var.value<QColor>(), isPreview);
        break;
    }
    case  EBorderWidth: {
        setPenWidth(var.toInt(), isPreview);
        break;
    }
    case  EStreakBeginStyle: {
        setPos1Style(ELinePosStyle(var.toInt()));
        break;
    }
    case  EStreakEndStyle: {
        setPos2Style(ELinePosStyle(var.toInt()));
        break;
    }
    case EPenWidth: {
        setPenWidth(var.toInt(), isPreview);
        break;
    }
    default:
        break;
    }

    LineBaseItem::setAttributionVar(attri, var, phase);
}

int LineItem::type() const
{
    return LineType;
}

void LineItem::doScaling(PageItemScalEvent *event)
{
    //preparePageItemGeometryChange();
    bool isSingle = isSingleSelected();
    if (isSingle) {
        int direc = event->pressedDirection();
        if (LineHandleNode::LinePos1 == direc) {
            //改变起点
            setLine(QLineF(event->pos(), line().p2()));
        } else if (LineHandleNode::LinePos2 == direc) {
            //改变第二个点
            setLine(QLineF(line().p1(), event->pos()));
        }
    } else {
        setLine(event->trans().map(_line));
    }
}

bool LineItem::testScaling(PageItemScalEvent *event)
{
    Q_UNUSED(event)
    return true;
}


Unit LineItem::getUnit(int reson) const
{
    Unit unit;

    unit.usage = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(LineUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    LineUnitData data;
    data.point1 = this->line().p1();
    data.point2 = this->line().p2();
    data.start_type = d_LineItem()->m_startType;
    data.end_type = d_LineItem()->m_endType;
    unit.data.data.setValue<LineUnitData>(data);
    return  unit;
}

void LineItem::loadUnit(const Unit &data)
{
    LineUnitData i = data.data.data.value<LineUnitData>();
    d_LineItem()->m_startType = i.start_type;
    d_LineItem()->m_endType = i.end_type;
    loadVectorData(data.head);
    setLine(QLineF(i.point1, i.point2));
}

void LineItem::setPos1Style(ELinePosStyle style)
{
    bool changed = (d_LineItem()->m_startType != style);
    d_LineItem()->m_startType = style;

    if (changed)
        updateShape();
}

ELinePosStyle LineItem::pos1Style() const
{
    return d_LineItem()->m_startType;
}

void LineItem::setPos2Style(ELinePosStyle style)
{
    bool changed = (d_LineItem()->m_endType != style);
    d_LineItem()->m_endType = style;
    if (changed)
        updateShape();
}

ELinePosStyle LineItem::pos2Style() const
{
    return d_LineItem()->m_endType;
}

bool LineItem::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)

    return !penStrokerShape().contains(posLocal);
    //return false;
}

QPainterPath LineItem::calOrgShapeBaseLine(const QLineF &line) const
{
    QPainterPath path, beginStylePath, endStylePath;
    QLineF outLine;
    d_LineItem()->calShowPath(path, outLine, beginStylePath, endStylePath);

    return path;
}

LineBaseItem::LineBaseItem(PageItem *parent): LineBaseItem(QLineF(), parent)
{

}

LineBaseItem::LineBaseItem(const QLineF &line, PageItem *parent): VectorItem(parent), _line(line)
{
}

QLineF LineBaseItem::line() const
{
    return _line;
}

void LineBaseItem::setLine(const QPointF &begin, const QPointF &end)
{
    setLine(QLineF(begin, end));
}

void LineBaseItem::setLine(const QLineF &line)
{
    if (line != this->line()) {
        _line = line;
        updateShape();
    }
}

QPainterPath LineBaseItem::getOrgShape() const
{
    return calOrgShapeBaseLine(line());
}

QPainterPath LineBaseItem::getPenStrokerShape() const
{
    return getPenStrokerPath(orgShape(), pen(), true, this->incLength(), false);
}

qreal LineBaseItem::incLength() const
{
    const int inccW = 10;
    PageView *parentView = nullptr;
    if (scene() != nullptr) {
        if (!scene()->views().isEmpty()) {
            parentView = dynamic_cast<PageView *>(scene()->views().first());
        }
    }


    qreal scal = (parentView == nullptr) ? 1.0 : parentView->getScale();
    return inccW / scal;
}

void LineItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    beginCheckIns(painter);

    const QPen curPen = this->paintPen();
    painter->setPen(curPen.width() == 0 ? Qt::NoPen : curPen);

    QPainterPath path, beginStylePath, endStylePath;
    QLineF outLine;
    d_LineItem()->calShowPath(path, outLine, beginStylePath, endStylePath);

    painter->save();
    if (d_LineItem()->beginStyleShouldSetBrush() && pen().width() != 0) {
        painter->setBrush(paintPen().color());
    }
    painter->drawPath(beginStylePath);
    painter->restore();

    painter->save();
    if (d_LineItem()->endStyleShouldSetBrush() && pen().width() != 0) {
        painter->setBrush(paintPen().color());
    }
    painter->drawPath(endStylePath);
    painter->restore();

    painter->drawLine(outLine);

    endCheckIns(painter);

    paintMutBoundingLine(painter, option);

    // 此处更新场景循环掉用，存在异常。保留此处注释提示直线更新异常Bug
    // scene()->update();
}

LineHandleNode::LineHandleNode(ENodeTpe tp, LineItem *parent):
    HandleNode(tp, parent)
{
    setCursor(innerCursor(Resize_LT));
}

LineItem *LineHandleNode::parentLineItem() const
{
    if (parentItem() == nullptr) {
        return nullptr;
    }
    return dynamic_cast<LineItem *>(parentItem());
}

void LineHandleNode::moveCenterTo(const QPointF &pos)
{
    auto size = boundingRect().size();

    auto posR = pos + QPointF(-size.width() / 2, -size.height() / 2);

    setPos(posR);
}

void LineHandleNode::setNodePos()
{
    auto lineItem = parentLineItem();
    if (nodeType() == LinePos1) {
        QPoint linepos1 = QPoint(lineItem->line().p1().x(), lineItem->line().p1().y());
        moveCenterTo(linepos1);
    } else if (nodeType() == LinePos2) {
        QPoint linepos2 = QPoint(lineItem->line().p2().x(), lineItem->line().p2().y());
        moveCenterTo(linepos2);
    }
}
