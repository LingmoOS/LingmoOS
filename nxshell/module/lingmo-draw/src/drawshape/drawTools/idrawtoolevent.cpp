// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "idrawtoolevent.h"
#include "cdrawscene.h"
#include "cgraphicsview.h"
#include "application.h"
#include <DApplication>

#include <QGraphicsSceneMouseEvent>

CDrawToolEvent::CDrawToolEvent(const QPointF &vPos,
                               const QPointF &scenePos,
                               const QPointF &globelPos,
                               PageScene *pScene)
{
    _pos[EViewportPos] = vPos;
    _pos[EScenePos]    = scenePos;
    _pos[EGlobelPos]   = globelPos;
    _scene             = pScene;
}

CDrawToolEvent::CDrawToolEvents CDrawToolEvent::fromQEvent(QEvent *event, PageScene *scene)
{
    CDrawToolEvents eList;
    CDrawToolEvent e;
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonDblClick: {
        QMouseEvent *msEvent = dynamic_cast<QMouseEvent *>(event);
        e._pos[EViewportPos] = scene->drawView()->viewport()->mapFromGlobal(msEvent->globalPos());
        e._pos[EGlobelPos]   = msEvent->globalPos();
        e._msBtns = msEvent->button() | msEvent->buttons();
        e._kbMods = msEvent->modifiers();
        e._orgEvent = event;
        e._scene    = scene;
        eList.insert(0, e);
        break;
    }
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseDoubleClick: {
        QGraphicsSceneMouseEvent *msEvent = dynamic_cast<QGraphicsSceneMouseEvent *>(event);
        e._pos[EViewportPos] = scene->drawView()->viewport()->mapFromGlobal(msEvent->screenPos());
        e._pos[EScenePos]    = msEvent->scenePos();
        e._pos[EGlobelPos]   = msEvent->screenPos();
        e._msBtns = msEvent->button() | msEvent->buttons();
        e._kbMods = msEvent->modifiers();
        e._orgEvent = event;
        e._scene    = scene;
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
            e._orgEvent = event;
            eList.insert(e.uuid(), e);
        }
        break;
    }
    default:
        e._pos[EViewportPos] = scene->drawView()->viewport()->mapFromGlobal(QCursor::pos());
        e._pos[EGlobelPos]   = QCursor::pos();
        e._pos[EScenePos]    = scene->drawView()->mapToScene(e._pos[EViewportPos].toPoint());
        e._orgEvent = event;
        e._scene    = scene;
        eList.insert(0, e);
        break;
    }
    return eList;
}

CDrawToolEvent CDrawToolEvent::fromQEvent_single(QEvent *event, PageScene *scene)
{
    auto list = fromQEvent(event, scene);
    if (!list.isEmpty())
        return list.first();
    return CDrawToolEvent();
}

CDrawToolEvent CDrawToolEvent::fromTouchPoint(const QTouchEvent::TouchPoint &tPos,
                                              PageScene *scene, QTouchEvent *eOrg)
{
    CDrawToolEvent e;
    e._pos[EViewportPos] = tPos.pos();
    e._pos[EScenePos]    = tPos.scenePos();
    e._pos[EGlobelPos]   = tPos.screenPos();
    e._uuid              = tPos.id();
    e._scene             = scene;
    e._kbMods            = eOrg == nullptr ? dApp->keyboardModifiers() : eOrg->modifiers();
    e._orgEvent = eOrg;
    return e;
}

QPointF CDrawToolEvent::pos(CDrawToolEvent::EPosType tp) const
{
    if (tp >= EScenePos && tp < PosTypeCount) {
        return _pos[tp];
    }
    return QPointF(0, 0);
}

Qt::MouseButtons CDrawToolEvent::mouseButtons() const
{
    return _msBtns;
}

Qt::KeyboardModifiers CDrawToolEvent::keyboardModifiers() const
{
    return _kbMods;
}

int CDrawToolEvent::uuid() const
{
    return _uuid;
}

CDrawToolEvent::EEventTp CDrawToolEvent::eventType()
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

QEvent *CDrawToolEvent::orgQtEvent()
{
    return _orgEvent;
}

PageScene *CDrawToolEvent::scene()
{
    return _scene;
}

PageView *CDrawToolEvent::view() const
{
    if (_scene != nullptr) {
        return _scene->drawView();
    }
    return nullptr;
}

bool CDrawToolEvent::isAccepted() const
{
    return _accept;
}

void CDrawToolEvent::setAccepted(bool b)
{
    _accept = b;
}

bool CDrawToolEvent::isPosXAccepted() const
{
    return _acceptPosX;
}

void CDrawToolEvent::setPosXAccepted(bool b)
{
    _acceptPosX = b;
}

bool CDrawToolEvent::isPosYAccepted() const
{
    return _acceptPosY;
}

void CDrawToolEvent::setPosYAccepted(bool b)
{
    _acceptPosY = b;
}
