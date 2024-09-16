// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALLDAYKEYLEFTDEAL_H
#define CALLDAYKEYLEFTDEAL_H

#include "ckeypressdealbase.h"

/**
 * @brief The CAllDayKeyLeftDeal class
 * 全体左方向键处理
 */
class CAllDayKeyLeftDeal : public CKeyPressDealBase
{
public:
    explicit CAllDayKeyLeftDeal(QGraphicsScene *scene = nullptr);

protected:
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CALLDAYKEYLEFTDEAL_H
