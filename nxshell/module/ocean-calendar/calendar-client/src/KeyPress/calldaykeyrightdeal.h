// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CALLDAYKEYRIGHTDEAL_H
#define CALLDAYKEYRIGHTDEAL_H

#include "ckeypressdealbase.h"

/**
 * @brief The CAllDayKeyRightDeal class
 *
 */
class CAllDayKeyRightDeal : public CKeyPressDealBase
{
public:
    explicit CAllDayKeyRightDeal(QGraphicsScene *scene = nullptr);

protected:
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CALLDAYKEYRIGHTDEAL_H
