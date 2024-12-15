// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CKEYPRESSDEALBASE_H
#define CKEYPRESSDEALBASE_H

#include <qnamespace.h>

class QGraphicsScene;
class CSceneBackgroundItem;
class CGraphicsScene;
class CKeyPressDealBase
{
public:
    explicit CKeyPressDealBase(Qt::Key key, QGraphicsScene *scene = nullptr);
    virtual ~CKeyPressDealBase();
    //获取注册的key
    Qt::Key getKey() const;
    bool dealEvent();
protected:
    virtual bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) = 0;

private:
    Qt::Key m_key;

protected:
    QGraphicsScene *m_scene;
};

#endif // CKEYPRESSDEALBASE_H
