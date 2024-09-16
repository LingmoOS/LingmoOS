// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CKEYUPDEAL_H
#define CKEYUPDEAL_H

#include "ckeypressdealbase.h"

/**
 * @brief The CKeyUpDeal class
 * 上方向键处理
 */
class CKeyUpDeal : public CKeyPressDealBase
{
public:
    explicit CKeyUpDeal(QGraphicsScene *scene = nullptr);

protected:
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CKEYUPDEAL_H
