// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CSCENETABKEYDEAL_H
#define CSCENETABKEYDEAL_H

#include "ckeypressdealbase.h"

/**
 * @brief The CSceneTabKeyDeal class
 * 场景tab切换处理
 */
class CSceneTabKeyDeal : public CKeyPressDealBase
{
public:
    explicit CSceneTabKeyDeal(QGraphicsScene *scene = nullptr);
protected:
    //焦点切换处理
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CSCENETABKEYDEAL_H
