// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "toolsceneevent.h"
#include "toolsceneevent_p.h"
#include "pagescene.h"
#include "pageview.h"
#include "drawboard.h"
#include "layeritem.h"
#include <QApplication>
#include <QGraphicsSceneMouseEvent>



ToolSceneEvent::ToolSceneEvent(const QPointF &vPos,
                               const QPointF &scenePos,
                               const QPointF &globelPos,
                               PageScene *pScene):
    ToolSceneEvent_d(new ToolSceneEvent_private(this, vPos, scenePos, globelPos, pScene))
{
}

ToolSceneEvent::CDrawToolEvents ToolSceneEvent::fromQEvent(QEvent *event, PageScene *scene)
{
    CDrawToolEvents eList;
    ToolSceneEvent e;
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *msEvent = dynamic_cast<QMouseEvent *>(event);
        e.d_ToolSceneEvent()->_pos[EViewportPos] = scene->firstPageView()->viewport()->mapFromGlobal(msEvent->globalPos());
        e.d_ToolSceneEvent()->_pos[EGlobelPos]   = msEvent->globalPos();
        e.d_ToolSceneEvent()->_msBtns = msEvent->button() | msEvent->buttons();
        e.d_ToolSceneEvent()->_kbMods = msEvent->modifiers();
        e.d_ToolSceneEvent()->_orgEvent = event;
        e.d_ToolSceneEvent()->_scene    = scene;
        eList.insert(0, e);
        break;
    }
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick: {
        QGraphicsSceneMouseEvent *msEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event);
        e.d_ToolSceneEvent()->_pos[EViewportPos] = scene->firstPageView()->viewport()->mapFromGlobal(msEvent->screenPos());
        e.d_ToolSceneEvent()->_pos[EScenePos]    = msEvent->scenePos();
        e.d_ToolSceneEvent()->_pos[EGlobelPos]   = msEvent->screenPos();
        e.d_ToolSceneEvent()->_msBtns = msEvent->button() | msEvent->buttons();
        e.d_ToolSceneEvent()->_kbMods = msEvent->modifiers();
        e.d_ToolSceneEvent()->_orgEvent = event;
        e.d_ToolSceneEvent()->_scene    = scene;
        eList.insert(0, e);
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);
        const QList<QTouchEvent::TouchPoint> lists = touchEvent->touchPoints();
        foreach (auto tPos, lists) {
            e = fromTouchPoint(tPos, scene);
            e.d_ToolSceneEvent()->_orgEvent = event;
            eList.insert(e.uuid(), e);
        }
        break;
    }
    default:
        e.d_ToolSceneEvent()->_pos[EViewportPos] = scene->firstPageView()->viewport()->mapFromGlobal(QCursor::pos());
        e.d_ToolSceneEvent()->_pos[EGlobelPos]   = QCursor::pos();
        e.d_ToolSceneEvent()->_pos[EScenePos]    = scene->firstPageView()->mapToScene(e.d_ToolSceneEvent()->_pos[EViewportPos].toPoint());
        e.d_ToolSceneEvent()->_orgEvent = event;
        e.d_ToolSceneEvent()->_scene    = scene;
        eList.insert(0, e);
        break;
    }
    return eList;
}

ToolSceneEvent ToolSceneEvent::fromQEvent_single(QEvent *event, PageScene *scene)
{
    auto list = fromQEvent(event, scene);
    if (!list.isEmpty())
        return list.first();
    return ToolSceneEvent();
}

ToolSceneEvent ToolSceneEvent::fromTouchPoint(const QTouchEvent::TouchPoint &tPos,
                                              PageScene *scene, QTouchEvent *eOrg)
{
    ToolSceneEvent e;
    e.d_ToolSceneEvent()->_pos[EViewportPos] = tPos.pos();
    e.d_ToolSceneEvent()->_pos[EScenePos]    = tPos.scenePos();
    e.d_ToolSceneEvent()->_pos[EGlobelPos]   = tPos.screenPos();
    e.d_ToolSceneEvent()->_uuid              = tPos.id();
    e.d_ToolSceneEvent()->_scene             = scene;
    e.d_ToolSceneEvent()->_kbMods            = eOrg == nullptr ? qApp->keyboardModifiers() : eOrg->modifiers();
    e.d_ToolSceneEvent()->_orgEvent = eOrg;
    return e;
}

QPointF ToolSceneEvent::scenePos() const
{
    return pos(EScenePos);
}

QPointF ToolSceneEvent::viewPos() const
{
    return pos(EViewportPos);
}

QPointF ToolSceneEvent::globelPos() const
{
    return pos(EGlobelPos);
}

QPointF ToolSceneEvent::currentLayerPos() const
{
    return toItemPos(d_ToolSceneEvent()->_scene->currentTopLayer());
}

QPointF ToolSceneEvent::toItemPos(PageItem *item) const
{
    if (item->scene() == d_ToolSceneEvent()->_scene)
        return item->mapFromScene(d_ToolSceneEvent()->_pos[EScenePos]);
    return QPointF(0, 0);
}

QPointF ToolSceneEvent::pos(ToolSceneEvent::EPosType tp) const
{
    if (tp >= EScenePos && tp < EInCurrentLayer) {
        return d_ToolSceneEvent()->_pos[tp];
    } else if (EInCurrentLayer == tp) {
        return d_ToolSceneEvent()->_scene->currentTopLayer()->mapFromScene(d_ToolSceneEvent()->_pos[EScenePos]);
    }
    return QPointF(0, 0);
}

void ToolSceneEvent::setPos(const QPointF &pos, EPosType tp)
{
    d_ToolSceneEvent()->_pos[tp] = pos;
}

Qt::MouseButtons ToolSceneEvent::mouseButtons() const
{
    return d_ToolSceneEvent()->_msBtns;
}

Qt::KeyboardModifiers ToolSceneEvent::keyboardModifiers() const
{
    return d_ToolSceneEvent()->_kbMods;
}

int ToolSceneEvent::uuid() const
{
    return d_ToolSceneEvent()->_uuid;
}

ToolSceneEvent::EEventTp ToolSceneEvent::eventType() const
{
    if (orgQtEvent() == nullptr)
        return EEventSimulated;

    EEventTp tp = EEventSimulated;

    switch (orgQtEvent()->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonDblClick:
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick: {
        tp = EMouseEvent;
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        tp = ETouchEvent;
        break;
    }
    default:
        break;
    }
    return tp;
}

QEvent *ToolSceneEvent::orgQtEvent()const
{
    return d_ToolSceneEvent()->_orgEvent;
}

PageScene *ToolSceneEvent::scene()const
{
    return d_ToolSceneEvent()->_scene;
}

PageView *ToolSceneEvent::view() const
{
    if (d_ToolSceneEvent()->_scene != nullptr) {
        return d_ToolSceneEvent()->_scene->firstPageView();
    }
    return nullptr;
}

bool ToolSceneEvent::isPressed() const
{
    if (eventType() == ETouchEvent) {
        return true;
    } else if (eventType() == EMouseEvent) {
        return mouseButtons() != Qt::NoButton;
    }
    return false;
}

bool ToolSceneEvent::isNormalPressed() const
{
    if (eventType() == ETouchEvent) {
        return true;
    } else if (eventType() == EMouseEvent) {
        return mouseButtons() & Qt::LeftButton;
    }
    return false;
}

QList<QGraphicsItem *> ToolSceneEvent::itemsUnderPressedPos()
{
    int incW = scene()->page()->borad()->touchFeelingEnhanceValue();
    if (d_ToolSceneEvent()->_itemsUnderPosDirty) {
        d_ToolSceneEvent()->_itemsUnderPos = eventType() == ToolSceneEvent::ETouchEvent ?
                                             this->scene()->items(QRectF(this->pos() - QPoint(incW, incW), QSizeF(2 * incW, 2 * incW))) :
                                             this->scene()->items(this->pos());

        d_ToolSceneEvent()->_itemsUnderPosDirty = false;
    }
    return d_ToolSceneEvent()->_itemsUnderPos;
}

PageItem *ToolSceneEvent::topPageItemUnderPressedPos()
{
    if (d_ToolSceneEvent()->_topPageItemDirty) {
        d_ToolSceneEvent()->_topPageItem = this->scene()->topPageItem(this->pos(), true,
                                                                      this->eventType() == ToolSceneEvent::ETouchEvent ?
                                                                      scene()->page()->borad()->touchFeelingEnhanceValue() : 0);

        d_ToolSceneEvent()->_topPageItemDirty = false;
    }
    return d_ToolSceneEvent()->_topPageItem;
}

QGraphicsItem *ToolSceneEvent::topItemUnderPressedPos()
{
    if (d_ToolSceneEvent()->_topItemDirty) {
        d_ToolSceneEvent()->_topItem = this->scene()->topItem(this->pos(),
                                                              QList<QGraphicsItem *>(), true, true, false, false, true,
                                                              this->eventType() == ToolSceneEvent::ETouchEvent ?
                                                              scene()->page()->borad()->touchFeelingEnhanceValue() : 0);

        d_ToolSceneEvent()->_topItemDirty = false;
    }
    return d_ToolSceneEvent()->_topItem;
}

ToolSceneEvent *ToolSceneEvent::firstEvent() const
{
    return d_ToolSceneEvent()->_firstEvent;
}

ToolSceneEvent *ToolSceneEvent::lastEvent() const
{
    return d_ToolSceneEvent()->_lastEvent;
}

int ToolSceneEvent::activedEventCount() const
{
    return d_ToolSceneEvent()->_leftEventCount;
}

//int ToolSceneEvent::movedDistance() const
//{
//    if (this == d_ToolSceneEvent()->_firstEvent)
//        return 0;

//    QPointF offset = this->pos(ToolSceneEvent::EViewportPos) - d_ToolSceneEvent()->_firstEvent->pos(ToolSceneEvent::EViewportPos);

//    return qRound(offset.manhattanLength());
//}

bool ToolSceneEvent::haveMoved() const
{
    return d_ToolSceneEvent()->moved;
}

bool ToolSceneEvent::isAccepted() const
{
    return d_ToolSceneEvent()->_accept;
}

void ToolSceneEvent::setAccepted(bool b)
{
    d_ToolSceneEvent()->_accept = b;
}

//bool ToolSceneEvent::isPosXAccepted() const
//{
//    return d_ToolSceneEvent()->_acceptPosX;
//}

void ToolSceneEvent::setPosXAccepted(bool b)
{
    d_ToolSceneEvent()->_acceptPosX = b;
}

//bool ToolSceneEvent::isPosYAccepted() const
//{
//    return d_ToolSceneEvent()->_acceptPosY;
//}

void ToolSceneEvent::setPosYAccepted(bool b)
{
    d_ToolSceneEvent()->_acceptPosY = b;
}
