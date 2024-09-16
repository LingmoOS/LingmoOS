// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cutitem.h"
#include "csizehandlerect.h"
//#include "frame/ccutwidget.h"
//#include "cdrawparamsigleton.h"
//#include "frame/cviewmanagement.h"
//#include "frame/cgraphicsview.h"
#include "pageview.h"
#include "pagescene.h"
#include "application.h"
#include "../../../service/dyncreatobject.h"

#include <QPushButton>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainterPath>
#include <QGraphicsProxyWidget>
#include <QMetaObject>

//const int CORNER_WITH = 20;
REGISTITEMCLASS(CutItem, CutType)

CutItem::CutItem(PageItem *parent)
    : PageItem(parent)
    , m_isFreeMode(false)
{
    initHandle();
}

CutItem::CutItem(const QRectF &rect, PageItem *parent)
    : PageItem(parent),
      m_topLeftPoint(rect.topLeft()),
      m_bottomRightPoint(rect.bottomRight())
    , m_isFreeMode(false)
{
    setOriginalRect(rect);
    initHandle();
}

CutItem::~CutItem()
{

}

SAttrisList CutItem::attributions()
{
    SAttrisList result;
    QList<QVariant> vars;
    vars << m_cutType << rect().size();
    SAttri attri(ECutToolAttri, vars);
    result << attri;
    return result;
}

QPainterPath CutItem::itemShape() const
{
    QPainterPath path;
    path.addRect(rect());
    return path;
}

bool CutItem::contains(const QPointF &point) const
{
    return itemShape().contains(point);
}

int CutItem::type() const
{
    return CutType;
}

void CutItem::setRect(const QRectF &rect)
{
    // 裁剪区域最小为10x10
    if (rect.width() < 10 || rect.height() < 10)
        return ;

    if (rect.width() > 10000 || rect.height() > 10000) {
        return;
    }

    preparePageItemGeometryChange();
    m_topLeftPoint = rect.topLeft();
    m_bottomRightPoint = rect.bottomRight();
    updateShape();
}

void CutItem::initHandle()
{
    // 子handles 用于处理重设大小
    for (int i = HandleNode::EInnerType::Resize_LT; i <= HandleNode::EInnerType::Resize_L; ++i) {
        auto shr = new CutItemNode(i, this);
        addHandleNode(shr);
        shr->setIconVisible(false);
    }
}

//void CutItem::updateHandlesGeometry()
//{
//    const QRectF &geom = this->rect();

//    const Handles::iterator hend =  handleNodes().end();
//    for (Handles::iterator it = handleNodes().begin(); it != hend; ++it) {
//        HandleNode *hndl = *it;
//        qreal w = hndl->boundingRect().width();
//        qreal h = hndl->boundingRect().height();
//        switch (hndl->nodeType()) {
//        case HandleNode::EInnerType::Resize_LT:
//            hndl->moveCenterTo(QPointF(geom.x() - w / 2, geom.y() - h / 2));
//            //hndl->move(geom.x() - w / 2, geom.y() - h / 2);
//            break;
//        case HandleNode::EInnerType::Resize_T:
//            hndl->moveCenterTo(QPointF(geom.x() + geom.width() / 2 - w / 2, geom.y() - h / 2));
//            //hndl->move(geom.x() + geom.width() / 2 - w / 2, geom.y() - h / 2);
//            break;
//        case HandleNode::EInnerType::Resize_RT:
//            hndl->moveCenterTo(QPointF(geom.x() + geom.width() - w / 2, geom.y() - h / 2));
//            //hndl->move(geom.x() + geom.width() - w / 2, geom.y() - h / 2);
//            break;
//        case HandleNode::EInnerType::Resize_R:
//            hndl->moveCenterTo(QPointF(geom.x() + geom.width() - w / 2, geom.y() + geom.height() / 2 - h / 2));
//            //hndl->move(geom.x() + geom.width() - w / 2, geom.y() + geom.height() / 2 - h / 2);
//            break;
//        case HandleNode::EInnerType::Resize_RB:
//            hndl->moveCenterTo(QPointF(geom.x() + geom.width() - w / 2, geom.y() + geom.height() - h / 2));
//            //hndl->move(geom.x() + geom.width() - w / 2, geom.y() + geom.height() - h / 2);
//            break;
//        case HandleNode::EInnerType::Resize_B:
//            hndl->moveCenterTo(QPointF(geom.x() + geom.width() / 2 - w / 2, geom.y() + geom.height() - h / 2));
//            //hndl->move(geom.x() + geom.width() / 2 - w / 2, geom.y() + geom.height() - h / 2);
//            break;
//        case HandleNode::EInnerType::Resize_LB:
//            hndl->moveCenterTo(QPointF(geom.x() - w / 2, geom.y() + geom.height() - h / 2));
//            //hndl->move(geom.x() - w / 2, geom.y() + geom.height() - h / 2);
//            break;
//        case HandleNode::EInnerType::Resize_L:
//            hndl->moveCenterTo(QPointF(geom.x() - w / 2, geom.y() + geom.height() / 2 - h / 2));
//            //hndl->move(geom.x() - w / 2, geom.y() + geom.height() / 2 - h / 2);
//            break;
//        default:
//            break;
//        }
//    }
//}

QRectF CutItem::rect() const
{
    return QRectF(m_topLeftPoint, m_bottomRightPoint);
}

QRectF CutItem::itemRect() const
{
    return rect();
}

void CutItem::move(QPointF beginPoint, QPointF movePoint)
{
    QPointF adjust = movePoint - beginPoint;
    setRect(rect().adjusted(adjust.x(), adjust.y(), adjust.x(), adjust.y()));
}

//QPainterPath CutItem::highLightPath()
//{
//    return QPainterPath();
//}

bool CutItem::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)
    return false;
}

//void CGraphicsCutItem::showControlRects(bool flag)
//{
//    setState(flag ? SelectionHandleActive : SelectionHandleOff);
//}

bool CutItem::isFreeMode() const
{
    return m_isFreeMode;
}

void CutItem::setIsFreeMode(bool isFreeMode)
{
    m_isFreeMode = isFreeMode;
    //showControlRects(true);
}


void CutItem::resizeCutSize(HandleNode::EInnerType dir,
                            const QPointF &prePoint,
                            const QPointF &point, QPointF *outAcceptPos)
{
    Q_UNUSED(outAcceptPos)
    //得到在自身坐标系内的当前鼠标位置
    QPointF preLocalPos = mapFromScene(prePoint);
    QPointF curLocalPos = mapFromScene(point);
    QRectF  curRect     = rect();
    qreal   qWHRadio    = getWHRadio();

    qreal moveX = (curLocalPos - preLocalPos).x();
    qreal moveY = (curLocalPos - preLocalPos).y();
    qreal adjust[4] = {0, 0, 0, 0};

    //bool getBorder = false;

    /*
     * fAdjustX 调整X轴上的位移
     * moveX 移动的值
     * currentRect 当前图元的rect(为了获取当前宽)
     * max，min 允许的最大值和最小值
     * directionX 移动的方向，需要传入-1或者1，1表示moveX是正值，-1表示是负值，请根据moveX的正负来传入
    */
    auto fAdjustX = [ = ](qreal & moveX, const QRectF & currentRect, qreal max, qreal min, int directionX = 1) {
        qreal wantedW = currentRect.width() + directionX * moveX;
        if (wantedW > max) {
            wantedW = max;
        } else if (wantedW < min) {
            wantedW = min;
        }
        moveX = (wantedW - currentRect.width()) / directionX;
    };
    /*
     * fAdjustY 调整Y轴上的位移
     * moveY 移动的值
     * currentRect 当前图元的rect(为了获取当前宽)
     * max，min 允许的最大值和最小值
     * directionY 移动的方向，需要传入-1或者1，1表示moveY是正值，-1表示是负值，请根据moveY的正负来传入
    */
    auto fAdjustY = [ = ](qreal & moveY, const QRectF & currentRect, qreal max, qreal min, int directionY = 1) {

        qreal wantedH = currentRect.height() + directionY * moveY;

        if (wantedH > max) {
            wantedH = max;
        } else if (wantedH < min) {
            wantedH = min;
        }
        moveY = (wantedH - currentRect.height()) / directionY;
    };
    /*
     * fAdjustBase 调整X，Y轴上的位移
     * moveX，moveY 移动的值
     * currentRect 当前图元的rect(为了获取当前宽)
     * baseXY 基于那个轴进行调整(0表示X轴，1表示Y轴)
     * directionX，directionY 移动的方向，需要传入-1或者1，1表示move是正值，-1表示是负值，请根据move的正负来传入
    */
    auto fAdjustBase = [ = ](qreal & moveX, qreal & moveY, const QRectF & currentRect, int baseXY = 0, int directionX = 1, int directionY = 1) {

        if (baseXY == 0) {

            qreal maxW = 10000;
            qreal minW = 10;

            qreal maxH = qWHRadio > 0 ? maxW / qWHRadio : 10000;
            qreal minH = qWHRadio > 0 ? minW / qWHRadio : 10;

            fAdjustX(moveX, currentRect, maxW, minW, directionX);
            if (qWHRadio > 0) {
                moveY = (moveX * directionX * directionY) / qWHRadio;
            }

            fAdjustY(moveY, currentRect, maxH, minH, directionY);

            if (qWHRadio > 0)
                moveX = moveY * qWHRadio / (directionX * directionY);
        } else {
            qreal maxH = 10000;
            qreal minH = 10;

            qreal maxW = qWHRadio > 0 ? maxH * qWHRadio : 10000;
            qreal minW = qWHRadio > 0 ? minH * qWHRadio : 10;

            fAdjustY(moveY, currentRect, maxH, minH, directionY);

            if (qWHRadio > 0) {
                moveX = moveY * directionX * directionY * qWHRadio;
            }

            fAdjustX(moveX, currentRect, maxW, minW, directionX);

            if (qWHRadio > 0)
                moveY = moveX / qWHRadio * directionX * directionY;
        }

    };

    switch (dir) {
    case HandleNode::EInnerType::Resize_T: {
        qreal mx = 0;
        qreal my = moveY;
        fAdjustBase(mx, my, curRect, 1, -1, -1);
        adjust[1] = moveY;

        adjust[0] = mx / 2;
        adjust[2] = -mx / 2;
        break;
    }
    case HandleNode::EInnerType::Resize_R: {
        qreal mx = moveX;
        qreal my = 0;
        fAdjustBase(mx, my, curRect, 0, 1, 1);
        adjust[2] = mx;
        adjust[1] = -my / 2;
        adjust[3] = my / 2;
        break;
    }
    case HandleNode::EInnerType::Resize_B: {
        qreal mx = 0;
        qreal my = moveY;
        fAdjustBase(mx, my, curRect, 1, 1, 1);
        adjust[3] = moveY;
        adjust[0] = -mx / 2;
        adjust[2] = mx / 2;
        break;
    }
    case HandleNode::EInnerType::Resize_L: {
        qreal mx = moveX;
        qreal my = 0;
        fAdjustBase(mx, my, curRect, 0, -1, -1);
        adjust[0] = mx;
        adjust[1] = my / 2;
        adjust[3] = -my / 2;
        break;
    }
    case HandleNode::EInnerType::Resize_LT: {
        qreal mx = moveX;
        qreal my = moveY;
        fAdjustBase(mx, my, curRect, qAbs(mx) < qAbs(my), -1, -1);
        adjust[0] = mx;
        adjust[1] = my;
        break;
    }
    case HandleNode::EInnerType::Resize_RT: {
        qreal mx = moveX;
        qreal my = moveY;
        fAdjustBase(mx, my, curRect, qAbs(mx) < qAbs(my), 1, -1);
        adjust[2] = mx;
        adjust[1] = my;
        break;
    }
    case HandleNode::EInnerType::Resize_RB: {
        qreal mx = moveX;
        qreal my = moveY;
        fAdjustBase(mx, my, curRect, qAbs(mx) < qAbs(my), 1, 1);
        adjust[2] = mx;
        adjust[3] = my;
        break;
    }
    case HandleNode::EInnerType::Resize_LB: {
        qreal mx = moveX;
        qreal my = moveY;
        fAdjustBase(mx, my, curRect, qAbs(mx) < qAbs(my), -1, 1);
        adjust[0] = mx;
        adjust[3] = my;
        break;
    }
    default: {
        break;
    }
    }

//    if (outAcceptPos != nullptr) {
//        if (getBorder) {
//            *outAcceptPos = (prePoint + QPointF(adjust[0] + adjust[2], adjust[1] + adjust[3]));
//        } else {
//            *outAcceptPos = point;
//        }
//    }

    curRect.adjust(adjust[0], adjust[1], adjust[2], adjust[3]);

    prepareGeometryChange();

    this->setRect(curRect);

    updateShape();
}

qreal CutItem::getWHRadio()
{
    qreal   qwhRadio    = -1;
    PageView *pView = pageView();
    if (pView != nullptr) {
        if (!isFreeMode()) {
            ECutType cutTp = getRatioType();
            switch (cutTp) {
            case cut_1_1:
                qwhRadio = 1.0;
                break;
            case cut_2_3:
                qwhRadio = 2.0 / 3.0;
                break;
            case cut_8_5:
                qwhRadio = 8.0 / 5.0;
                break;
            case cut_16_9:
                qwhRadio = 16.0 / 9.0;
                break;
            case cut_original:
                qwhRadio = scene()->sceneRect().width() / scene()->sceneRect().height();
                break;
            default:
                break;
            }
        }
    }
    return qwhRadio;
}

Handles CutItem::nodes()
{
    return handleNodes();
}

void CutItem::setRatioType(ECutType type)
{
    m_cutType = type;

    QRectF rect = this->scene()->sceneRect();
    QPointF centerPos = rect.center();
    qreal w = rect.width();
    qreal h = rect.height();
    QPointF topLeft;
    QPointF rightBottom;
    qreal bigW = w;
    qreal bigH = h;

    if (type == cut_original) {
        bigW = m_originalInitRect.width();
        bigH = m_originalInitRect.height();
    } else {
        switch (type) {
        case cut_1_1:
            bigH = bigW;
            break;
        case cut_2_3:
            bigH = bigW * 3 / 2;
            break;
        case cut_8_5:
            bigH = bigW * 5 / 8;
            break;
        case cut_16_9:
            bigH = bigW * 9 / 16;
            break;
        default:
            break;

        }

        qreal scale = (bigW / bigH);

        if (bigW > w || bigH > h) {
            if (scale >= (w / h)) {
                bigW = w;
                bigH = (bigW / scale);
            } else {
                bigH = h;
                bigW = (bigH * scale);
            }
        }
    }

    topLeft = centerPos - QPointF(bigW / 2, bigH / 2);
    rightBottom = centerPos + QPointF(bigW / 2, bigH / 2);
    this->setRect(QRectF(topLeft, rightBottom));

    setIsFreeMode(false);
}

ECutType CutItem::getRatioType() const
{
    return m_cutType;
}

void CutItem::setSize(int w, int h)
{
    setRect(QRectF(rect().x(), rect().y(), w, h));
}

void CutItem::setOriginalInitRect(const QRectF &size)
{
    m_originalInitRect = size;
}

void CutItem::setOriginalRect(const QRectF &size)
{
    m_originalRect = size;
}

QRectF CutItem::originalRect() const
{
    return m_originalRect;
}

//QVariant CutItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
//{
//    Q_UNUSED(change);
//    if (scene() != nullptr && !scene()->views().isEmpty()) {
//        scene()->views().first()->viewport()->update();
//    }
//    return PageItem::itemChange(change, value);
//}

void CutItem::paintSelf(QPainter *painter, const QStyleOptionGraphicsItem *option)
{
    Q_UNUSED(option)
    //updateHandlesGeometry();

    painter->setClipping(false);

    //获取系统主题活动色
    QColor activeColor = pageView()->pageScene()->systemThemeColor();

    painter->save();
    //设置画笔参数
    qreal penWidth = 1.0 / painter->worldTransform().m11();
    QRectF rct = rect().adjusted(penWidth, penWidth, -penWidth, -penWidth);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    QColor penColor = QColor("#EDEDED");
    pen.setColor(penColor);
    pen.setWidthF(penWidth * 3);
    painter->setPen(pen);

    //画三等分矩形的直线
    drawTrisectorRect(painter);
    painter->restore();

    painter->save();
    //绘制边框投影
    QPen rectPen(pen);
    rectPen.setStyle(Qt::SolidLine);
    QColor rectColor(activeColor);
    rectColor.setAlpha(26);
    rectPen.setColor(rectColor);
    rectPen.setWidthF(penWidth * 5); //绘制边框投影是画笔的5倍宽度
    painter->setPen(rectPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rct);
    painter->restore();

    painter->save();
    //绘制边框矩形
    rectPen.setStyle(Qt::DashLine);
    rectPen.setColor(activeColor);
    rectPen.setWidthF(penWidth);
    painter->setPen(rectPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rct);
    painter->restore();

    painter->save();
    //绘制拐角区域
    drawFourConner(painter);
    painter->restore();

    painter->save();
    //绘制交叉填充背景
    QRectF sceneRct = scene()->sceneRect();
    QRectF itemRct  = mapToScene(rect()).boundingRect();
    QRegion r1(sceneRct.toRect());
    QRegion r2(itemRct.toRect());
    QRegion r3 = r2.subtracted(r1);
    QPainterPath path;
    path.addRegion(r3);

    QColor background(activeColor);
    background.setAlpha(26);
    painter->setPen(Qt::NoPen);
    painter->setBrush(background);
    painter->drawPath(path);

    QPen penaaa(Qt::red);
    penaaa.setWidth(4);
    penaaa.setBrush(QBrush(Qt::red));
    painter->setPen(penaaa);
    painter->restore();
}


void CutItem::drawTrisectorRect(QPainter *painter)
{
    qreal penWidth = 1.0 / painter->worldTransform().m11();
    QPainterPath path;
    QRectF rct = rect().adjusted(penWidth, penWidth, -penWidth, -penWidth);
    path.moveTo(rct.x(), rct.y() + rct.height() / 3);
    path.lineTo(rct.x() + rct.width(), rct.y() + rct.height() / 3);

    path.moveTo(rct.x(), rct.y() + rct.height() / 3 * 2);
    path.lineTo(rct.x() + rct.width(), rct.y() + rct.height() / 3 * 2);

    path.moveTo(rct.x() + rct.width() / 3, rct.y());
    path.lineTo(rct.x() + rct.width() / 3, rct.y() + rct.height());

    path.moveTo(rct.x() + rct.width() / 3 * 2, rct.y());
    path.lineTo(rct.x() + rct.width() / 3 * 2, rct.y() + rct.height());

    painter->drawPath(path);
}

void CutItem::drawFourConner(QPainter *painter/*, QPainterPath &path, const int penWidth*/)
{
    qreal penWidth = 1.0 / painter->worldTransform().m11();
    QRectF rct = rect().adjusted(penWidth, penWidth, -penWidth, -penWidth);

    const qreal minlenth = 24;
    qreal showW = rect().width() * pageView()->getScale();
    qreal showH = rect().height() * pageView()->getScale();

    if (showW < minlenth || showH < minlenth) {
        //当裁剪区域小于拐角图片进行画笔绘制
        QPen pen(painter->pen());
        pen.setWidthF(1.0);
        pen.setColor(QColor("#EDEDED"));
        pen.setStyle(Qt::SolidLine);

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(rct);

        QLineF topLine   = QLineF(rct.topLeft(), rct.topRight());
        QLineF botLine   = QLineF(rct.bottomLeft(), rct.bottomRight());
        QLineF leftLine  = QLineF(rct.topLeft(), rct.bottomLeft());
        QLineF rightLine = QLineF(rct.topRight(), rct.bottomRight());

        painter->drawLine(topLine.p1(), topLine.center());
        painter->drawLine(rightLine.p1(), rightLine.center());
        painter->drawLine(botLine.center(), botLine.p2());
        painter->drawLine(leftLine.center(), leftLine.p2());

        painter->drawLine(topLine.center(), topLine.p2());
        painter->drawLine(rightLine.center(), rightLine.p2());
        painter->drawLine(botLine.center(), botLine.p1());
        painter->drawLine(leftLine.center(), leftLine.p1());
    } else {

        painter->save();

        //获取当前使用坐标变换的缩放值设置画笔宽度
        int pixWidth = qRound(20.0 /*/ painter->worldTransform().m11()*/);
        int offset = qRound(2.0 /*/ painter->worldTransform().m11()*/);
        int offsetWidth = qRound(18.0 /*/ painter->worldTransform().m11()*/);

        //绘制左上角
        auto painterOffset  = pageView()->mapFromScene(sceneBoundingRect().topLeft());
        auto painterOffset2 =  pageView()->mapFromScene(sceneBoundingRect().bottomRight());
        rct = QRectF(painterOffset, painterOffset2);
        painter->resetTransform();
        QPixmap cornerPixmap = QIcon::fromTheme("selection_topLeft").pixmap(QSize(pixWidth, pixWidth));
        QRectF cornerRect = QRectF(painterOffset + QPointF(-offset, -offset), painterOffset + QPointF(pixWidth, pixWidth));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));


        //绘制右上角
        //painter->translate(rect().width(), 0);
        cornerPixmap = QIcon::fromTheme("selection_topRight").pixmap(QSize(pixWidth, pixWidth));
        cornerRect = QRectF(QPointF(painterOffset2.x() - offsetWidth, painterOffset.y() - offset), QSizeF(pixWidth, pixWidth));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));

        //绘制右下角
        cornerPixmap = QIcon::fromTheme("selection_bottomRight").pixmap(QSize(pixWidth, pixWidth));
        cornerRect = QRectF(rct.bottomRight() + QPointF(-pixWidth, -pixWidth), rct.bottomRight() + QPointF(offset, offset));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));

        //绘制左下角
        cornerPixmap = QIcon::fromTheme("selection_bottomLeft").pixmap(QSize(pixWidth, pixWidth));
        cornerRect = QRectF(QPointF(rct.x() - offset, rct.y() + rct.height() - offsetWidth), QSizeF(pixWidth, pixWidth));
        painter->drawPixmap(cornerRect, cornerPixmap, QRectF(0, 0, cornerPixmap.width(), cornerPixmap.height()));

        painter->restore();
    }
}
bool CutItemNode::isVisbleCondition() const
{
    return (parentPageItem() != nullptr && parentPageItem()->pageGroup() == nullptr);
}
