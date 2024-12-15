// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scenecutitem.h"
#include "csizehandlerect.h"
#include "globaldefine.h"
#include "pagecontext.h"
#include "pageview.h"
#include "pagescene.h"

#include <QPushButton>
#include <QPainter>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QGraphicsProxyWidget>

REGISTITEMCLASS(SceneCutItem, CutType)
SceneCutItem::SceneCutItem(PageItem *parent)
    : PageItem(parent)
    , m_isFreeMode(false)
{
    initHandle();
    //Q_UNUSED(nodes());
}

SceneCutItem::SceneCutItem(const QRectF &rect, PageItem *parent)
    : PageItem(parent),
      m_topLeftPoint(rect.topLeft()),
      m_bottomRightPoint(rect.bottomRight())
    , m_isFreeMode(false)
{
    setOriginalRect(rect);
}

SceneCutItem::~SceneCutItem()
{

}

SAttrisList SceneCutItem::attributions()
{
    SAttrisList result;
    QList<QVariant> vars;
    vars << m_cutType << itemRect().size();
    SAttri attri(ECutToolAttri, vars);
    result << attri;
    return result;
}

bool SceneCutItem::contains(const QPointF &point) const
{
    return shape().contains(point);
}

int SceneCutItem::type() const
{
    return CutType;
}

void SceneCutItem::setRect(const QRectF &rect)
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
    //updateHandlesGeometry();

    if (pageView() != nullptr) {
        pageView()->setFocus();
    }
}

void SceneCutItem::initHandle()
{
    for (int i = HandleNode::Resize_LT; i <= HandleNode::Resize_L; ++i) {
        HandleNode *shr = nullptr;
        shr = new HandleNode(HandleNode::EInnerType(i), this);
        shr->setIconVisible(false);
        addHandleNode(shr);
    }
}

//void SceneCutItem::updateHandlesGeometry()
//{
//    const QRectF &geom = this->itemRect();

//    const Handles::iterator hend =  m_handles.end();
//    for (Handles::iterator it = m_handles.begin(); it != hend; ++it) {
//        HandleNode *hndl = *it;
//        qreal w = hndl->boundingRect().width();
//        qreal h = hndl->boundingRect().height();
//        switch (hndl->nodeType()) {
//        case HandleNode::Resize_LT:
//            hndl->setPos(geom.x() - w / 2, geom.y() - h / 2);
//            break;
//        case HandleNode::Resize_T:
//            hndl->setPos(geom.x() + geom.width() / 2 - w / 2, geom.y() - h / 2);
//            break;
//        case HandleNode::Resize_RT:
//            hndl->setPos(geom.x() + geom.width() - w / 2, geom.y() - h / 2);
//            break;
//        case HandleNode::Resize_R:
//            hndl->setPos(geom.x() + geom.width() - w / 2, geom.y() + geom.height() / 2 - h / 2);
//            break;
//        case HandleNode::Resize_RB:
//            hndl->setPos(geom.x() + geom.width() - w / 2, geom.y() + geom.height() - h / 2);
//            break;
//        case HandleNode::Resize_B:
//            hndl->setPos(geom.x() + geom.width() / 2 - w / 2, geom.y() + geom.height() - h / 2);
//            break;
//        case HandleNode::Resize_LB:
//            hndl->setPos(geom.x() - w / 2, geom.y() + geom.height() - h / 2);
//            break;
//        case HandleNode::Resize_L:
//            hndl->setPos(geom.x() - w / 2, geom.y() + geom.height() / 2 - h / 2);
//            break;
//        default:
//            break;
//        }
//    }
//}

QRectF SceneCutItem::itemRect() const
{
    return QRectF(m_topLeftPoint, m_bottomRightPoint);
}

void SceneCutItem::move(QPointF beginPoint, QPointF movePoint)
{
    QPointF adjust = movePoint - beginPoint;
    setRect(itemRect().adjusted(adjust.x(), adjust.y(), adjust.x(), adjust.y()));
}

QPainterPath SceneCutItem::highLightPath() const
{
    return QPainterPath();
}

bool SceneCutItem::isPosPenetrable(const QPointF &posLocal)
{
    Q_UNUSED(posLocal)
    return false;
}

bool SceneCutItem::isFreeMode() const
{
    return m_isFreeMode;
}

void SceneCutItem::setIsFreeMode(bool isFreeMode)
{
    m_isFreeMode = isFreeMode;
    //showControlRects(true);
}

void SceneCutItem::resizeCutSize(HandleNode::EInnerType dir,
                                 const QPointF &prePoint,
                                 const QPointF &point, QPointF *outAcceptPos)
{
    //得到在自身坐标系内的当前鼠标位置
    QPointF preLocalPos = mapFromScene(prePoint);
    QPointF curLocalPos = mapFromScene(point);
    QRectF  curRect     = itemRect();
    qreal   qWHRadio    = getWHRadio();

    qreal moveX = (curLocalPos - preLocalPos).x();
    qreal moveY = (curLocalPos - preLocalPos).y();
    qreal adjust[4] = {0, 0, 0, 0};

    bool getBorder = false;

    qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
    qreal minH = (qWHRadio < 1.0 && qWHRadio > 0.0) ? 10 / qWHRadio : 10;

    //根据dir的位置判断应该改变到的大小
    switch (dir) {
    case HandleNode::Resize_T: {
        adjust[1] = moveY;

        //边界检查
        if (curRect.height() - adjust[1] < minH) {
            adjust[1] = curRect.height() - minH;
            getBorder = true;
        }

        if (qWHRadio > 0) {
            adjust[2] = -adjust[1] * qWHRadio;
        }
        break;
    }

    case HandleNode::Resize_B: {
        adjust[3] = moveY;

        //边界检查
        if (curRect.height() + adjust[3] < minH) {
            adjust[3] = minH - curRect.height();
            getBorder = true;
        }

        if (qWHRadio > 0) {
            adjust[2] = adjust[3] * qWHRadio;
        }
        break;
    }
    case HandleNode::Resize_L: {
        adjust[0] = moveX;

        //边界检查
        if (curRect.width() - adjust[0] < minW) {
            adjust[0] = curRect.width() - minW;
            getBorder = true;
        }

        if (qWHRadio > 0) {
            adjust[3] = -adjust[0] / qWHRadio;
        }

        break;
    }
    case HandleNode::Resize_R: {
        adjust[2] = moveX;

        //边界检查
        if (curRect.width() + adjust[2] < minW) {
            adjust[2] = minW - curRect.width();
            getBorder = true;
        }

        if (qWHRadio > 0) {
            adjust[3] = adjust[2] / qWHRadio;
            getBorder = true;
        }
        break;
    }
    case HandleNode::Resize_RT: {
        if (qWHRadio > 0) {
            if (qAbs(moveX) > qAbs(moveY)) {
                adjust[2] = moveX;

                //边界检查
                //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
                if (curRect.width() + adjust[2] < minW) {
                    adjust[2] = minW - curRect.width();
                    getBorder = true;
                }

                adjust[1] = -adjust[2] / qWHRadio;
            } else {
                adjust[1] = moveY;

                //边界检查
                //qreal minH = qWHRadio < 1.0 ? 10 / qWHRadio : 10;
                if (curRect.height() - adjust[1] < minH) {
                    adjust[1] = curRect.height() - minH;
                    getBorder = true;
                }

                adjust[2] = -adjust[1] * qWHRadio;
            }
        } else {

            adjust[2] = moveX;
            adjust[1] = moveY;

            //边界检查
            //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
            if (curRect.width() + adjust[2] < minW) {
                adjust[2] = minW - curRect.width();
                getBorder = true;
            }

            //qreal minH = qWHRadio < 1.0 ? 10 / qWHRadio : 10;
            if (curRect.height() - adjust[1] < minH) {
                adjust[1] = curRect.height() - minH;
                getBorder = true;
            }
        }

        break;
    }
    case HandleNode::Resize_LT: {
        if (qWHRadio > 0) {
            if (qAbs(moveX) > qAbs(moveY)) {
                adjust[0] = moveX;
                //边界检查
                //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
                if (curRect.width() - adjust[0] < minW) {
                    adjust[0] = curRect.width() - minW;
                    getBorder = true;
                }
                adjust[1] = adjust[0] / qWHRadio;
            } else {
                adjust[1] = moveY;
                //边界检查
                //qreal minH = qWHRadio < 1.0 ? 10 / qWHRadio : 10;
                if (curRect.height() - adjust[1] < minH) {
                    adjust[1] = curRect.height() - minH;
                    getBorder = true;
                }
                adjust[0] = adjust[1] * qWHRadio;
            }
        } else {
            adjust[0] = moveX;
            adjust[1] = moveY;

            //边界检查
            //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
            if (curRect.width() - adjust[0] < minW) {
                adjust[0] = curRect.width() - minW;
                getBorder = true;
            }

            //qreal minH = qWHRadio < 1.0 ? 10 / qWHRadio : 10;
            if (curRect.height() - adjust[1] < minH) {
                adjust[1] = curRect.height() - minH;
                getBorder = true;
            }
        }
        break;
    }

    case HandleNode::Resize_RB: {
        if (qWHRadio > 0) {
            if (qAbs(moveX) > qAbs(moveY)) {
                adjust[2] = moveX;

                //边界检查
                //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
                if (curRect.width() + adjust[2] < minW) {
                    adjust[2] = minW - curRect.width();
                    getBorder = true;
                }

                adjust[3] = adjust[2] / qWHRadio;
            } else {
                adjust[3] = moveY;

                //边界检查
                //qreal minH = qWHRadio < 1.0 ? 10 / qWHRadio : 10;
                if (curRect.height() + adjust[3] < minH) {
                    adjust[3] = minH - curRect.height();
                    getBorder = true;
                }

                adjust[2] = adjust[3] * qWHRadio;
            }
        } else {
            adjust[2] = moveX;
            adjust[3] = moveY;

            //边界检查
            //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
            if (curRect.width() + adjust[2] < minW) {
                adjust[2] = minW - curRect.width();
                getBorder = true;
            }

            //qreal minH = qWHRadio < 1.0 ? 10 / qWHRadio : 10;
            if (curRect.height() + adjust[3] < minH) {
                adjust[3] = minH - curRect.height();
                getBorder = true;
            }
        }

        break;
    }

    case HandleNode::Resize_LB: {
        if (qWHRadio > 0) {
            if (qAbs(moveX) > qAbs(moveY)) {
                adjust[0] = moveX;

                //边界检查
                //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
                if (curRect.width() - adjust[0] < minW) {
                    adjust[0] = curRect.width() - minW;
                    getBorder = true;
                }

                adjust[3] = -adjust[0] / qWHRadio;
            } else {
                adjust[3] = moveY;

                //边界检查
                //qreal minH = qWHRadio < 1.0 ? 10 / qWHRadio : 10;
                if (curRect.height() + adjust[3] < minH) {
                    adjust[3] = minH - curRect.height();
                    getBorder = true;
                }

                adjust[0] = -adjust[3] * qWHRadio;
            }
        } else {
            adjust[0] = moveX;
            adjust[3] = moveY;

            //边界检查
            //qreal minW = qWHRadio > 1.0 ? 10 * qWHRadio : 10;
            if (curRect.width() - adjust[0] < minW) {
                adjust[0] = curRect.width() - minW;
                getBorder = true;
            }

            //qreal minH = qWHRadio < 1.0 ? 10 * qWHRadio : 10;
            if (curRect.height() + adjust[3] < minH) {
                adjust[3] = minH - curRect.height();
                getBorder = true;
            }

        }
        break;
    }
    default:
        break;
    }

    if (outAcceptPos != nullptr) {
        if (getBorder) {
            *outAcceptPos = (prePoint + QPointF(adjust[0] + adjust[2], adjust[1] + adjust[3]));
        } else {
            *outAcceptPos = point;
        }
    }

    curRect.adjust(adjust[0], adjust[1], adjust[2], adjust[3]);

    this->setRect(curRect);

    //updateHandlesGeometry();
}

qreal SceneCutItem::getWHRadio()
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

void SceneCutItem::setRatioType(ECutType type)
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

SceneCutItem::ECutType SceneCutItem::getRatioType() const
{
    return m_cutType;
}

void SceneCutItem::setSize(int w, int h)
{
    setRect(QRectF(itemRect().x(), itemRect().y(), w, h));
}

void SceneCutItem::setOriginalInitRect(const QRectF &size)
{
    m_originalInitRect = size;
}

void SceneCutItem::setOriginalRect(const QRectF &size)
{
    m_originalRect = size;
}

QRectF SceneCutItem::originalRect() const
{
    return m_originalRect;
}

QVariant SceneCutItem::pageItemChange(int change, const QVariant &value)
{
    Q_UNUSED(change)
    if (scene() != nullptr && !scene()->views().isEmpty()) {
        scene()->views().first()->viewport()->update();
    }
    return PageItem::pageItemChange(change, value);
}

void SceneCutItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    //updateHandlesGeometry();

    painter->setClipping(false);

    //获取系统主题活动色
    QColor activeColor = pageScene()->systemThemeColor();

    painter->save();
    //设置画笔参数
    qreal penWidth = 1.0 / painter->worldTransform().m11();
    QRectF rct = itemRect().adjusted(penWidth, penWidth, -penWidth, -penWidth);
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
    QRectF itemRct  = mapToScene(itemRect()).boundingRect();
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
    painter->restore();
}

void SceneCutItem::drawTrisectorRect(QPainter *painter)
{
    qreal penWidth = 1.0 / painter->worldTransform().m11();
    QPainterPath path;
    QRectF rct = itemRect().adjusted(penWidth, penWidth, -penWidth, -penWidth);
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

void SceneCutItem::drawFourConner(QPainter *painter/*, QPainterPath &path, const int penWidth*/)
{
    qreal penWidth = 1.0 / painter->worldTransform().m11();
    if (itemRect().width() < 15.0 || itemRect().height() < 15.0) {
        QRectF rct = itemRect().adjusted(penWidth, penWidth, -penWidth, -penWidth);
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
        auto rct = QRect(painterOffset, painterOffset2);
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
