// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CGRAPHICSITEMEVENT_H
#define CGRAPHICSITEMEVENT_H

#include <QPointF>
#include "globaldefine.h"

class PageItem;

class DRAWLIB_EXPORT PageItemEvent
{
public:
    enum EItemType {EMove, EScal, ERot, EBlur, EUnKnow};

    PageItemEvent(EItemType tp = EUnKnow,
                  const QPointF &oldPos = QPointF(),
                  const QPointF &pos    = QPointF());

    virtual ~PageItemEvent() {}

    inline QPointF oldPos() const {return _oldPos;}
    void   setOldPos(const QPointF &pos);

    inline QPointF pos() const {return _pos;}
    void   setPos(const QPointF &pos);

    inline QPointF beginPos() const {return _beginPos;}
    void   setBeginPos(const QPointF &pos);

    inline QPointF centerPos() const {return _centerPos;}
    void   setCenterPos(const QPointF &pos);

    inline QSizeF orgSize() const {return _orgSz;}
    void   setOrgSize(const QSizeF &sz);

    inline EItemType type() const {return this->_tp;}

    inline EChangedPhase eventPhase() const {return this->_phase;}
    void   setEventPhase(EChangedPhase ph);

    inline int toolEventType() const {return _orgToolEventTp;}
    void   setToolEventType(int tp) {_orgToolEventTp = tp;}

    inline int pressedDirection() const {return _pressedDirection;}
    void   setPressedDirection(int direction) {_pressedDirection = direction;}

    inline bool isAccept() const {return _accept;}

// Cppcheck检测函数没有使用到
//    void   setAccept(bool b);

    inline bool isPosXAccept() const {return _acceptPosX;}
//    void   setPosXAccept(bool b);

    inline bool isPosYAccept() const {return _acceptPosY;}
//    void   setPosYAccept(bool b);

    inline QPointF   offset() const {return _pos - _oldPos;}
    inline QPointF   totalOffset() const {return _pos - _beginPos;}


//    CGraphItemEvent *driverEvent();
    void             setDriverEvent(PageItemEvent *event);

    void      setItem(PageItem *pItem);
    PageItem      *item();

    QTransform trans();
    void       setTrans(const QTransform &trans);
    void       updateTrans();

    PageItemEvent *creatTransDuplicate(const QTransform &tran, const QSizeF &newOrgSz);

protected:
    virtual PageItemEvent *newInstace();
    virtual void transAllPosTo(const QTransform &tran);

    /**
     * @brief reCalTransform 重新计算转换矩阵
     */
    virtual bool reCalTransform(QTransform &outTrans);

protected:
    EItemType  _tp;
    QPointF    _oldPos;
    QPointF    _pos;

    QPointF    _beginPos;

    QPointF    _centerPos;
    QSizeF     _orgSz;

    EChangedPhase _phase = EChanged;

    QTransform    _trans;

    bool          _transDirty = true;

    bool          _accept = true;
    bool          _acceptPosX = true;
    bool          _acceptPosY = true;

    int           _orgToolEventTp = 0;
    int           _pressedDirection = -1;


    PageItemEvent *_driverEvent = nullptr;
    PageItem  *_pItem = nullptr;
public:
    QPointF    _oldScenePos;
    QPointF    _scenePos;
    QPointF    _sceneBeginPos;
    QPointF    _sceneCenterPos;
};

/**
 * @brief PageItemMoveEvent 图元移动事件
 */
class DRAWLIB_EXPORT PageItemMoveEvent: public PageItemEvent
{
public:
    using PageItemEvent::PageItemEvent;

protected:
    PageItemEvent *newInstace() override;
    /**
     * @brief reCalTransform 计算移动事件的转换矩阵
     */
    bool reCalTransform(QTransform &outTrans) override;
};

/**
 * @brief PageItemScalEvent 图元缩放调整事件
 */
class DRAWLIB_EXPORT PageItemScalEvent: public PageItemEvent
{
public:

    using PageItemEvent::PageItemEvent;

// Cppcheck检测函数没有使用到
//    inline bool isXTransBlocked() const;

    void   setXTransBlocked(bool b);

// Cppcheck检测函数没有使用到
//    inline bool isYTransBlocked() const;

    void   setYTransBlocked(bool b);

// Cppcheck检测函数没有使用到
//    inline bool isXNegtiveOffset() const;

    void   setXNegtiveOffset(bool b);

// Cppcheck检测函数没有使用到
//    inline bool isYNegtiveOffset() const;

    void   setYNegtiveOffset(bool b);

// Cppcheck检测函数没有使用到
//    inline bool isKeepOrgRadio() const;

    void   setKeepOrgRadio(bool b);


    void      setMayResultPolygon(const QPolygonF rect);
//    QPolygonF mayResultPolygon();

protected:

    PageItemEvent *newInstace() override;
    /**
     * @brief reCalTransform 计算缩放事件的转换矩阵
     */
    bool reCalTransform(QTransform &outTrans) override;

protected:
    bool          _blockXTrans = false;
    bool          _blockYTrans = false;
    bool          _isXNegtiveOffset = false;
    bool          _isYNegtiveOffset = false;
    bool          _isKeepOrgRadio = false;

    qreal         _offsetRadioX = 1.0;
    qreal         _offsetRadioY = 1.0;

    QPolygonF        _driverMayResultRect; //在场景中的坐标系
};

/**
 * @brief PageItemRotEvent 图元旋转事件
 */

class DRAWLIB_EXPORT PageItemRotEvent: public PageItemEvent
{
public:
    using PageItemEvent::PageItemEvent;
    PageItemRotEvent(qreal rota = 0, const QPointF &center = QPointF(0, 0));
    inline qreal usingAngle() const
    { return m_dUsingAngle; }
    // 返回是否使用直接角度值
    inline bool isUsingAngle() const
    { return m_bUsingAngle; }
protected:

    PageItemEvent *newInstace() override;

    /**
     * @brief reCalTransform 计算缩放事件的转换矩阵
     */
    bool reCalTransform(QTransform &outTrans) override;
public:
    qreal   m_dUsingAngle = 0.0;        // 用于直接使用角度值的计算
    bool    m_bUsingAngle = false;      // 用于标志是否使用直接角度值
};
#endif // CGRAPHICSITEMEVENT_H
