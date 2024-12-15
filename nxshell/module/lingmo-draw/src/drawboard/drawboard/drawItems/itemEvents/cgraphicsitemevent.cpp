// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cgraphicsitemevent.h"
#include <QDebug>

PageItemEvent::PageItemEvent(PageItemEvent::EItemType tp,
                             const QPointF &oldPos,
                             const QPointF &pos)
    : _tp(tp), _oldPos(oldPos), _pos(pos)
{

}

void PageItemEvent::setOldPos(const QPointF &pos)
{
    _oldPos = pos;
    _transDirty = true;
}

void PageItemEvent::setPos(const QPointF &pos)
{
    _pos = pos;
    _transDirty = true;
}

void PageItemEvent::setBeginPos(const QPointF &pos)
{
    _beginPos = pos;
    _transDirty = true;
}

void PageItemEvent::setCenterPos(const QPointF &pos)
{
    _centerPos = pos;
    _transDirty = true;
}

void PageItemEvent::setOrgSize(const QSizeF &sz)
{
    _orgSz = sz;
    _transDirty = true;
}

void PageItemEvent::setEventPhase(EChangedPhase ph)
{
    _phase = ph;
}

// Cppcheck检测函数没有使用到
//void CGraphItemEvent::setAccept(bool b)
//{
//    _accept = b;
//}

//void CGraphItemEvent::setPosXAccept(bool b)
//{
//    _acceptPosX = b;
//}

//void CGraphItemEvent::setPosYAccept(bool b)
//{
//    _acceptPosY = b;
//}

//CGraphItemEvent *CGraphItemEvent::driverEvent()
//{
//    return _driverEvent;
//}

void PageItemEvent::setDriverEvent(PageItemEvent *event)
{
//    if (event != nullptr) {
//        qWarning() << "event======================== " << event->type();
//    }
    _driverEvent = event;
}

void PageItemEvent::setItem(PageItem *pItem)
{
    _pItem = pItem;
}

PageItem *PageItemEvent::item()
{
    return _pItem;
}

void PageItemEvent::updateTrans()
{
    QTransform trans;
    if (reCalTransform(trans)) {
        _trans = trans;
    }
    _transDirty = false;
}

PageItemEvent *PageItemEvent::creatTransDuplicate(const QTransform &tran, const QSizeF &newOrgSz)
{
    auto p = newInstace();
    p->setOrgSize(newOrgSz);
    p->transAllPosTo(tran);
    return p;
}

PageItemEvent *PageItemEvent::newInstace()
{
    auto p = new PageItemEvent;
    *p = *this;
    return p;
}

QTransform PageItemEvent::trans()
{
    if (_transDirty) {
        updateTrans();
    }
    return _trans;
}

void PageItemEvent::setTrans(const QTransform &trans)
{
    _trans = trans;
    _transDirty = false;
}

//CGraphItemEvent *CGraphItemEvent::transToEvent(const QTransform &tran, const QSizeF &newOrgSz)
//{
//    CGraphItemEvent event = (this->type());
//    event.setEventPhase(this->eventPhase());

//    event.setPos(tran.map(this->pos()));
//    event.setOldPos(tran.map(this->oldPos()));
//    event.setBeginPos(tran.map(this->beginPos()));
//    event.setCenterPos(tran.map(this->centerPos()));
//    event.setOrgSize(newOrgSz);


//    event._blockXTrans = this->_blockXTrans;
//    event._blockYTrans = this->_blockYTrans;
//    event._isXNegtiveOffset = this->_isXNegtiveOffset;
//    event._isYNegtiveOffset = this->_isYNegtiveOffset;

//    event._pressedDirection = this->_pressedDirection;
//    event._orgToolEventTp = this->_orgToolEventTp;

//    event._oldScenePos = this->_oldScenePos;
//    event._scenePos = this->_scenePos;
//    event._sceneBeginPos = this->_sceneBeginPos;
//    event._sceneCenterPos = this->_sceneCenterPos;

//    event.updateTrans();

//    return event;
//}

void PageItemEvent::transAllPosTo(const QTransform &tran)
{
    this->setPos(tran.map(this->pos()));
    this->setOldPos(tran.map(this->oldPos()));
    this->setBeginPos(tran.map(this->beginPos()));
    this->setCenterPos(tran.map(this->centerPos()));
    this->updateTrans();
}

bool PageItemEvent::reCalTransform(QTransform &outTrans)
{
    Q_UNUSED(outTrans);
    return false;
}


PageItemEvent *PageItemMoveEvent::newInstace()
{
    auto p = new PageItemMoveEvent;
    *p = *this;
    return p;
}

bool PageItemMoveEvent::reCalTransform(QTransform &outTrans)
{
    //仅仅是一个平移
    QPointF move = this->pos() - this->oldPos();
    outTrans = QTransform::fromTranslate(move.x(), move.y());
    return true;
}

// Cppcheck检测函数没有使用到
//bool CGraphItemScalEvent::isXTransBlocked() const
//{
//    return _blockXTrans;
//}

void PageItemScalEvent::setXTransBlocked(bool b)
{
    _blockXTrans = b;
    _transDirty = true;
}

// Cppcheck检测函数没有使用到
//bool CGraphItemScalEvent::isYTransBlocked() const
//{
//    return _blockYTrans;
//}

void PageItemScalEvent::setYTransBlocked(bool b)
{
    _blockYTrans = b;
    _transDirty = true;
}

// Cppcheck检测函数没有使用到
//bool CGraphItemScalEvent::isXNegtiveOffset() const
//{
//    return _isXNegtiveOffset;
//}

void PageItemScalEvent::setXNegtiveOffset(bool b)
{
    _isXNegtiveOffset = b;
    _transDirty = true;
}

// Cppcheck检测函数没有使用到
//bool CGraphItemScalEvent::isYNegtiveOffset() const
//{
//    return _isYNegtiveOffset;
//}

void PageItemScalEvent::setYNegtiveOffset(bool b)
{
    _isYNegtiveOffset = b;
    _transDirty = true;
}

// Cppcheck检测函数没有使用到
//bool CGraphItemScalEvent::isKeepOrgRadio() const
//{
//    return _isKeepOrgRadio;
//}

void PageItemScalEvent::setKeepOrgRadio(bool b)
{
    _isKeepOrgRadio = b;
    _transDirty = true;
}

void PageItemScalEvent::setMayResultPolygon(const QPolygonF rect)
{
    _driverMayResultRect = rect;
}

//QPolygonF CGraphItemScalEvent::mayResultPolygon()
//{
//    return _driverMayResultRect;
//}

PageItemEvent *PageItemScalEvent::newInstace()
{
    auto p = new PageItemScalEvent;
    *p = *this;
    return p;
}

bool PageItemScalEvent::reCalTransform(QTransform &outTrans)
{
    QTransform trans;

    if (!_orgSz.isValid()) {
        return false;
    }

    const QPointF centerPos = this->centerPos();

    //qWarning() << "current pos = " << this->pos() << "old pos = " << this->oldPos();
    const qreal offsetX = (_isXNegtiveOffset ? -1 : 1) * this->offset().x();
    const qreal offsetY = (_isYNegtiveOffset ? -1 : 1) * this->offset().y();

    qreal sX = _blockXTrans ? 1.0 : (offsetX + _orgSz.width()) / _orgSz.width();
    qreal sY = _blockYTrans ? 1.0 : (offsetY + _orgSz.height()) / _orgSz.height();

    const qreal minW = 1.0;
    if (_orgSz.width() * sX < minW) {
        sX = minW / _orgSz.width();
        _acceptPosX = false;
    }
    const qreal minH = 1.0;
    if (_orgSz.height() * sY < minH) {
        sY = minH / _orgSz.height();
        _acceptPosY = false;
    }

    if (_isKeepOrgRadio) {
        if (qAbs(offsetY) > qAbs(offsetX)) {
            sX = sY;
        } else {
            sY = sX;
        }
    }

    trans.translate(centerPos.x(), centerPos.y());
    trans.scale(sX, sY);
    trans.translate(-centerPos.x(), -centerPos.y());

    outTrans = trans;

    return true;
}

PageItemRotEvent::PageItemRotEvent(qreal rota, const QPointF &center):
    PageItemEvent(PageItemEvent::ERot)
    , m_dUsingAngle(rota)
    , m_bUsingAngle(true)
{
    setEventPhase(EChanged);
    setCenterPos(center);
    setOldPos(center + QPointF(0, -2));

    QLineF line(center, oldPos());
    line.setAngle(line.angle() - rota);
    setPos(line.p2());


//    PageItemRotEvent rot(PageItemEvent::ERot);
//    rot.setEventPhase(phase);
//    rot.setToolEventType(decideValue(event->uuid()));
//    rot.setPressedDirection(d_SelectTool()->_extraTp);
//    rot._scenePos = event->pos();
//    rot._oldScenePos = event->lastEvent()->pos();
//    rot._sceneBeginPos = event->firstEvent()->pos();
//    //分发事件
//    if (PageScene::isPageItem(d_SelectTool()->_extraItem) /*|| _extraItem == event->scene()->selectGroup()*/) {
//        PageItem *pBzItem = d_SelectTool()->_extraItem;
//        rot.setPos(pBzItem->mapFromScene(event->pos()));
//        rot.setOldPos(pBzItem->mapFromScene(event->lastEvent()->pos()));
//        rot.setOrgSize(pBzItem->orgRect().size());
//        rot.setCenterPos(pBzItem->orgRect().center());
//        rot._sceneCenterPos = pBzItem->mapToScene(rot.centerPos());
//        if (phase == EChangedBegin) {
//            pBzItem->operatingBegin(&rot);
//        } else if (phase == EChangedUpdate) {
//            pBzItem->operating(&rot);
//        } else if (phase == EChangedFinished) {
//            pBzItem->operatingEnd(&rot);
//        }
//    }

}

PageItemEvent *PageItemRotEvent::newInstace()
{
    auto p = new PageItemRotEvent;
    *p = *this;
    return p;
}

bool PageItemRotEvent::reCalTransform(QTransform &outTrans)
{
    auto prePos = oldPos();
    auto pos    = this->pos();
    auto center = this->centerPos();

    QLineF l1 = QLineF(center, prePos);
    QLineF l2 = QLineF(center, pos);
    qreal angle = l2.angle() - l1.angle();
    QTransform trans;
    trans.translate(center.x(), center.y());
    trans.rotate(-angle);
    trans.translate(-center.x(), -center.y());
    outTrans = trans;
    return true;
}
