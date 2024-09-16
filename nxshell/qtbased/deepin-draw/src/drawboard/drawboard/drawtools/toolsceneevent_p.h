// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOOLSCENEEVENT_P_H
#define TOOLSCENEEVENT_P_H
#include "toolsceneevent.h"

class ToolSceneEvent::ToolSceneEvent_private
{
public:
    ToolSceneEvent_private(ToolSceneEvent *q, const QPointF &vPos,
                           const QPointF &scenePos,
                           const QPointF &globelPos,
                           PageScene *pScene): _q(q), _pos{scenePos, vPos, globelPos},
        _scene(pScene),
        _itemsUnderPosDirty(true),
        _topItemDirty(true),
        _topPageItemDirty(true)
    {}

    QPointF _pos[PosTypeCount] = {QPointF(0, 0)};

    Qt::MouseButtons       _msBtns   = Qt::NoButton;
    Qt::KeyboardModifiers  _kbMods   = Qt::NoModifier;
    PageScene             *_scene    = nullptr;
    int                    _uuid     = 0;
    QEvent                *_orgEvent = nullptr;
    bool _accept = true;
    bool _acceptPosX = true;
    bool _acceptPosY = true;

    QList<QGraphicsItem *> _itemsUnderPos;
    int _itemsUnderPosDirty: 1;

    QGraphicsItem *_topItem = nullptr;
    int _topItemDirty: 1;

    PageItem *_topPageItem = nullptr;
    int _topPageItemDirty: 1;

    ToolSceneEvent *_firstEvent  = nullptr;
    ToolSceneEvent *_lastEvent   = nullptr;

    int _leftEventCount = 1;

    bool moved = false;

    ToolSceneEvent *_q;
};

#endif // TOOLSCENEEVENT_P_H
