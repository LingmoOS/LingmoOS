// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicslineitem.h"
#include "frame/cviewmanagement.h"
#include "frame/cgraphicsview.h"
#include "cgraphicsitemevent.h"
#include "cgraphicsitemselectedmgr.h"
#include "cattributeitemwidget.h"
#include "../../../service/dyncreatobject.h"

#include <DSvgRenderer>

#include <QPen>
#include <QPainter>
#include <QPainterPath>
#include <QPointF>
#include <QtMath>
#include <QDebug>

DTK_USE_NAMESPACE

REGISTITEMCLASS(CGraphicsLineItem, LineType)
CGraphicsLineItem::CGraphicsLineItem(QGraphicsItem *parent)
    : CGraphicsItem(parent)
{
    initLine();
}

CGraphicsLineItem::CGraphicsLineItem(const QLineF &line, QGraphicsItem *parent)
    : CGraphicsItem(parent)
    , m_line(line)
{
    initLine();
}

CGraphicsLineItem::CGraphicsLineItem(const QPointF &p1, const QPointF &p2, QGraphicsItem *parent)
    : CGraphicsItem(parent)
{
    setLine(p1.x(), p1.y(), p2.x(), p2.y(), true);
    initLine();
}

CGraphicsLineItem::CGraphicsLineItem(qreal x1, qreal y1, qreal x2, qreal y2, QGraphicsItem *parent)
    : CGraphicsItem(parent)
    , m_line(x1, y1, x2, y2)
{
    initLine();
}

CGraphicsLineItem::~CGraphicsLineItem()
{
//    m_handles.clear();
}

DrawAttribution::SAttrisList CGraphicsLineItem::attributions()
{
    DrawAttribution::SAttrisList result;
    result << DrawAttribution::SAttri(DrawAttribution::EPenColor, pen().color())
           << DrawAttribution::SAttri(DrawAttribution::EBorderWidth,  pen().width())
           << DrawAttribution::SAttri(1775)
           << DrawAttribution::SAttri(DrawAttribution::EStreakBeginStyle, getLineStartType())
           << DrawAttribution::SAttri(DrawAttribution::EStreakEndStyle,  getLineEndType());
    return result;
}

void CGraphicsLineItem::setAttributionVar(int attri, const QVariant &var, int phase)
{
    bool isPreview = (phase == EChangedBegin || phase == EChangedUpdate);
    switch (attri) {
    case DrawAttribution::EPenColor: {
        setPenColor(var.value<QColor>(), isPreview);
        break;
    }
    case DrawAttribution::EBorderWidth: {
        setPenWidth(var.toInt(), isPreview);
        break;
    }
    case DrawAttribution::EStreakBeginStyle: {
        setLineStartType(ELineType(var.toInt()));
        break;
    }
    case DrawAttribution::EStreakEndStyle: {
        setLineEndType(ELineType(var.toInt()));
        break;
    }
    default:
        break;
    };
}

int CGraphicsLineItem::type() const
{
    return LineType;
}

void CGraphicsLineItem::doScaling(CGraphItemScalEvent *event)
{
    prepareGeometryChange();

    auto pG = bzGroup(false);
    bool isSingle = (pG == nullptr || pG->count() == 1);
    if (isSingle) {
        CSizeHandleRect::EDirection direc = CSizeHandleRect::EDirection(event->pressedDirection());
        if (CSizeHandleRect::LeftTop == direc) {
            //改变起点
            m_line.setP1(event->pos());
        } else if (CSizeHandleRect::RightBottom == direc) {
            //改变第二个点
            m_line.setP2(event->pos());
        }
    } else {
        m_line = event->trans().map(m_line);
    }
    updateShape();
}

bool CGraphicsLineItem::testScaling(CGraphItemScalEvent *event)
{
    Q_UNUSED(event)
    return true;
}

QRectF CGraphicsLineItem::rect() const
{
    QPainterPath path(m_dRectline.p1());
    path.lineTo(m_dRectline.p2());
    path.addPath(m_startPath);
    path.addPath(m_endPath);

    return path.controlPointRect().normalized();
}

QLineF CGraphicsLineItem::line() const
{
    return m_line;
}

void CGraphicsLineItem::setLine(const QLineF &line, bool init)
{
    prepareGeometryChange();
    m_line = line;

    updateShape();

    if (init) {
        //CGraphicsItem::updateGeometry();
    } else {
        updateHandlesGeometry();
    }
}

void CGraphicsLineItem::setLine(const QPointF &p1, const QPointF &p2, bool init)
{
    setLine(p1.x(), p1.y(), p2.x(), p2.y(), init);
}

void CGraphicsLineItem::setLine(qreal x1, qreal y1, qreal x2, qreal y2, bool init)
{
    setLine(QLineF(x1, y1, x2, y2), init);
}

CGraphicsUnit CGraphicsLineItem::getGraphicsUnit(EDataReason reson) const
{
    CGraphicsUnit unit;

    unit.reson = reson;

    unit.head.dataType = this->type();
    unit.head.dataLength = sizeof(SGraphicsLineUnitData);
    unit.head.pen = this->pen();
    unit.head.brush = this->brush();
    unit.head.pos = this->pos();
    unit.head.rotate = /*this->rotation()*/this->drawRotation();
    unit.head.zValue = this->zValue();
    unit.head.trans = this->transform();

    unit.data.pLine = new SGraphicsLineUnitData();
    unit.data.pLine->point1 = this->line().p1();
    unit.data.pLine->point2 = this->line().p2();
    unit.data.pLine->start_type = m_startType;
    unit.data.pLine->end_type = m_endType;

    return  unit;
}

void CGraphicsLineItem::loadGraphicsUnit(const CGraphicsUnit &data)
{
    if (data.data.pLine != nullptr) {
        m_startType = data.data.pLine->start_type;
        m_endType = data.data.pLine->end_type;
        m_line = QLineF(data.data.pLine->point1, data.data.pLine->point2);
    }
    loadHeadData(data.head);

    updateShape();
    updateHandlesGeometry();
}

//int CGraphicsLineItem::getQuadrant() const
//{
//    int nRet = 1;
//    if (m_line.p2().x() - m_line.p1().x() > 0.0001 && m_line.p2().y() - m_line.p1().y() < 0.0001) {
//        nRet = 1;
//    } else if (m_line.p2().x() - m_line.p1().x() > 0.0001 && m_line.p2().y() - m_line.p1().y() > 0.0001) {
//        nRet = 2;
//    } else if (m_line.p2().x() - m_line.p1().x() < 0.0001 && m_line.p2().y() - m_line.p1().y() > 0.0001) {
//        nRet = 3;
//    } else if (m_line.p2().x() - m_line.p1().x() < 0.0001 && m_line.p2().y() - m_line.p1().y() < 0.0001) {
//        nRet = 4;
//    }
//    return nRet;
//}

void CGraphicsLineItem::setLineStartType(ELineType type)
{
    bool changed = (m_startType != type);
    m_startType = type;

    if (changed)
        updateShapeRecursion();
}

ELineType CGraphicsLineItem::getLineStartType() const
{
    return m_startType;
}

void CGraphicsLineItem::setLineEndType(ELineType type)
{
    bool changed = (m_endType != type);
    m_endType = type;

    if (changed)
        updateShapeRecursion();
}

ELineType CGraphicsLineItem::getLineEndType() const
{
    return m_endType;
}

void CGraphicsLineItem::updateHandlesGeometry()
{
    qreal penwidth = this->pen().widthF();
    for (Handles::iterator it = m_handles.begin(); it != m_handles.end(); ++it) {
        CSizeHandleRect *hndl = *it;

        if (!this->isSelected() || this->isMutiSelected() || bzGroup() != nullptr) {
            hndl->hide();
            continue;
        }

        hndl->show();

        qreal w = hndl->boundingRect().width();
        qreal h = hndl->boundingRect().height();
        switch (hndl->dir()) {
        case CSizeHandleRect::LeftTop:
            hndl->move(m_dRectline.p1().x() - w / 2, m_dRectline.p1().y() - h / 2);
            break;
        case CSizeHandleRect::RightBottom:
            hndl->move(m_dRectline.p2().x() - w / 2, m_dRectline.p2().y() - h / 2);
            break;
        case CSizeHandleRect::Rotation: {
            // 以下代码没有实际作用
            QPointF centerPos = (m_dRectline.p1() + m_dRectline.p2()) / 2;


            if (qAbs(m_dRectline.p2().x() - m_dRectline.p1().x()) < 0.0001) {
                hndl->move(m_dRectline.p1().x() - h - penwidth, centerPos.y());
            } else {
                qreal k = 0;
                qreal ang = 0;
                k = -(m_dRectline.p2().y() - m_dRectline.p1().y()) / (m_dRectline.p2().x() - m_dRectline.p1().x());
                ang = atan(k);

                //增加线宽的长度防止缩放造成位置不正确
                qreal x = qAbs((h + penwidth) * sin(ang));
                qreal y = qAbs((h + penwidth) * cos(ang));
                //第一象限
                if (m_dRectline.p2().x() - m_dRectline.p1().x() > 0.0001 && m_dRectline.p2().y() - m_dRectline.p1().y() < 0.0001) {
                    hndl->move(centerPos.x() - w / 2 - x, centerPos.y() - h / 2 - y);
                } else if (m_dRectline.p2().x() - m_dRectline.p1().x() > 0.0001 && m_dRectline.p2().y() - m_dRectline.p1().y() > 0.0001) {
                    hndl->move(centerPos.x() - w / 2 + x, centerPos.y() - h / 2 - y);
                } else if (m_dRectline.p2().x() - m_dRectline.p1().x() < 0.0001 && m_dRectline.p2().y() - m_dRectline.p1().y() > 0.0001) {
                    hndl->move(centerPos.x() - w / 2 + x, centerPos.y() - h / 2 + y);
                } else if (m_dRectline.p2().x() - m_dRectline.p1().x() < 0.0001 && m_dRectline.p2().y() - m_dRectline.p1().y() < 0.0001) {
                    hndl->move(centerPos.x() - w / 2 - x, centerPos.y() - h / 2 + y);
                }
            }
            break;
        }
        default:
            break;
        }
    }
}

void CGraphicsLineItem::updateShape()
{
    calcVertexes();
    CGraphicsItem::updateShape();
}

void CGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    //qWarning() << "bounding ========== " << this->boundingRect();

    updateHandlesGeometry();

    painter->setRenderHint(QPainter::Antialiasing, true);

    const QPen curPen = paintPen();
    painter->setPen(curPen.width() == 0 ? Qt::NoPen : curPen);

    beginCheckIns(painter);

    painter->setBrush(Qt::NoBrush);
    if (curPen.width()) {
        if (m_startType == soildArrow || m_startType == soildRing) {
            painter->setBrush(QBrush(QColor(curPen.color())));
        }
    }
    painter->drawPath(m_startPath);

    painter->setBrush(Qt::NoBrush);
    if (curPen.width()) {
        if (m_endType == soildArrow || m_endType == soildRing) {
            painter->setBrush(QBrush(QColor(curPen.color())));
        }
    }
    painter->drawPath(m_endPath);

    painter->drawLine(m_line);

    endCheckIns(painter);
}

QVariant CGraphicsLineItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) {
        updateHandlesGeometry();
    }
    return CGraphicsItem::itemChange(change, value);
}

bool CGraphicsLineItem::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)
//    if (pen().color().alpha() == 0 || pen().width() == 0 || pen().widthF() == .0) {
//        return true;
//    }
    return false;
}

void CGraphicsLineItem::initLine()
{
    //CGraphicsItem::initHandle();

    leftTop = new CSizeHandleRect(this, CSizeHandleRect::LeftTop);
    rightBottom = new CSizeHandleRect(this, CSizeHandleRect::RightBottom);

    updateHandlesGeometry();

    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);

    updateShape();
}

void CGraphicsLineItem::initHandle()
{
    clearHandle();

    m_handles.reserve(CSizeHandleRect::None);

    m_handles.push_back(leftTop);
    m_handles.push_back(rightBottom);

    updateHandlesGeometry();

    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);

    //calcVertexes();
    updateShape();
}

void CGraphicsLineItem::drawStart()
{
    QPointF prePoint = m_line.p1();
    QPointF currentPoint = m_line.p2();
    if (prePoint == currentPoint) {
        return;
    }

    QLineF templine(m_line.p2(), m_line.p1());

    QLineF v = templine.unitVector();
    v.setLength(10 + pen().width() * 3); //改变单位向量的大小，实际就是改变箭头长度
    v.translate(QPointF(templine.dx(), templine.dy()));

    QLineF n = v.normalVector(); //法向量
    n.setLength(n.length() * 0.5); //这里设定箭头的宽度
    QLineF n2 = n.normalVector().normalVector(); //两次法向量运算以后，就得到一个反向的法向量

    QPointF p1 = v.p2();
    QPointF p2 = n.p2();
    QPointF p3 = n2.p2();

    //减去一个箭头的宽度
    QPointF diffV = p1 - m_line.p1();
    p1 -= diffV;
    p2 -= diffV;
    p3 -= diffV;

    switch (m_startType) {
    case noneLine: {
        m_startPath = QPainterPath(p1);
        m_dRectline.setP1(p1);
        break;
    }
    case normalArrow: {
        p1 += diffV;
        p2 += diffV;
        p3 += diffV;
        m_startPath = QPainterPath(p1);
        m_startPath.lineTo(p2);
        m_startPath.moveTo(p1);
        m_startPath.lineTo(p3);
        m_startPath.moveTo(p1);
        QPointF center = (p2 + p3) / 2;
        m_startPath.lineTo(center);
        m_startPath.moveTo(p1);
        m_dRectline.setP1(p1);
        break;
    }
    case soildArrow: {
        p1 += diffV;
        p2 += diffV;
        p3 += diffV;
        m_startPath = QPainterPath(p1);
        m_startPath.lineTo(p3);
        m_startPath.lineTo(p2);
        m_startPath.lineTo(p1);
        m_dRectline.setP1(p1);
        break;
    }
    case normalRing: {
        qreal radioWidth = this->pen().width() * 2;
        QPointF center;
        qreal yOff = qSin(m_line.angle() / 180 * M_PI) * radioWidth;
        qreal xOff = qCos(m_line.angle() / 180 * M_PI) * radioWidth;
        center = m_line.p1() + QPointF(-xOff, yOff);
        QRectF ecliRect(center + QPointF(-radioWidth, -radioWidth), QSizeF(2 * radioWidth, 2 * radioWidth));
        m_startPath = QPainterPath(center + QPointF(radioWidth, 0));
        m_startPath.arcTo(ecliRect, 0, 360);
        m_dRectline.setP1(center + QPointF(-xOff, yOff));
        break;
    }
    case soildRing: {
        qreal radioWidth = this->pen().width() * 2;
        QPointF center;
        qreal yOff = qSin(m_line.angle() / 180 * M_PI) * radioWidth;
        qreal xOff = qCos(m_line.angle() / 180 * M_PI) * radioWidth;
        center = m_line.p1() + QPointF(-xOff, yOff);
        QRectF ecliRect(center + QPointF(-radioWidth, -radioWidth), QSizeF(2 * radioWidth, 2 * radioWidth));
        m_startPath = QPainterPath(center + QPointF(radioWidth, 0));
        m_startPath.arcTo(ecliRect, 0, 360);
        m_dRectline.setP1(center + QPointF(-xOff, yOff));
        break;
    }
    }
}

void CGraphicsLineItem::drawEnd()
{
    QPointF prePoint = m_line.p1();
    QPointF currentPoint = m_line.p2();
    if (prePoint == currentPoint) {
        return;
    }

    QLineF v = m_line.unitVector();
    v.setLength(10 + pen().width() * 3); //改变单位向量的大小，实际就是改变箭头长度
    v.translate(QPointF(m_line.dx(), m_line.dy()));

    QLineF n = v.normalVector(); //法向量
    n.setLength(n.length() * 0.5); //这里设定箭头的宽度
    QLineF n2 = n.normalVector().normalVector(); //两次法向量运算以后，就得到一个反向的法向量

    QPointF p1 = v.p2();
    QPointF p2 = n.p2();
    QPointF p3 = n2.p2();

    //减去一个箭头的宽度
    QPointF diffV = p1 - m_line.p2();
    //    QPointF diffV = p1 - m_line.p1();
    p1 -= diffV;
    p2 -= diffV;
    p3 -= diffV;

    // 绘制终点
    switch (m_endType) {
    case noneLine: {
        m_endPath = QPainterPath(p1);
        m_dRectline.setP2(p1);
        break;
    }
    case normalArrow: {
        p1 += diffV;
        p2 += diffV;
        p3 += diffV;
        m_endPath = QPainterPath(p1);
        m_endPath.lineTo(p2);
        m_endPath.moveTo(p1);
        m_endPath.lineTo(p3);
        m_endPath.moveTo(p1);
        QPointF center = (p2 + p3) / 2;
        m_endPath.lineTo(center);
        m_endPath.moveTo(p1);
        m_dRectline.setP2(p1);
        break;
    }
    case soildArrow: {
        p1 += diffV;
        p2 += diffV;
        p3 += diffV;
        m_endPath = QPainterPath(p1);
        m_endPath.lineTo(p3);
        m_endPath.lineTo(p2);
        m_endPath.lineTo(p1);
        m_dRectline.setP2(p1);
        break;
    }
    case normalRing: {
        qreal radioWidth = this->pen().width() * 2;
        QPointF center;
        qreal yOff = qSin(m_line.angle() / 180 * M_PI) * radioWidth;
        qreal xOff = qCos(m_line.angle() / 180 * M_PI) * radioWidth;
        center = m_line.p2() + QPointF(xOff, -yOff);
        QRectF ecliRect(center + QPointF(-radioWidth, -radioWidth), QSizeF(2 * radioWidth, 2 * radioWidth));
        m_endPath = QPainterPath(center + QPointF(radioWidth, 0));
        m_endPath.arcTo(ecliRect, 0, 360);
        m_dRectline.setP2(center + + QPointF(xOff, -yOff));
        break;
    }
    case soildRing: {
        qreal radioWidth = this->pen().width() * 2;
        QPointF center;
        qreal yOff = qSin(m_line.angle() / 180 * M_PI) * radioWidth;
        qreal xOff = qCos(m_line.angle() / 180 * M_PI) * radioWidth;
        center = m_line.p2() + QPointF(xOff, -yOff);
        QRectF ecliRect(center + QPointF(-radioWidth, -radioWidth), QSizeF(2 * radioWidth, 2 * radioWidth));
        m_endPath = QPainterPath(center + QPointF(radioWidth, 0));
        m_endPath.arcTo(ecliRect, 0, 360);
        m_dRectline.setP2(center + + QPointF(xOff, -yOff));
        break;
    }
    }
}

void CGraphicsLineItem::calcVertexes()
{
    prepareGeometryChange();

    // 绘制起点
    drawStart();

    // 绘制终点
    drawEnd();

    updateHandlesGeometry();

    // 更新画布区域
    if (curView() != nullptr)
        curView()->viewport()->update();
}

QPainterPath CGraphicsLineItem::getHighLightPath()
{
    QPainterPath path(m_line.p1());
    path.lineTo(m_line.p2());
    path.addPath(m_startPath);
    path.addPath(m_endPath);
    return path;
}

QPainterPath CGraphicsLineItem::getSelfOrgShape() const
{
    QPainterPath path;

//    if (this->curView() == nullptr)
//        return path;

    if (m_line == QLineF())
        return path;

    path.moveTo(m_line.p1());
    path.lineTo(m_line.p2());

    path.addPath(m_startPath);
    path.addPath(m_endPath);

    return path;
}
