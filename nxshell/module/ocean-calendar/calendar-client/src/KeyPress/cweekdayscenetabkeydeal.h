// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CWEEKDAYSCENETABKEYDEAL_H
#define CWEEKDAYSCENETABKEYDEAL_H

#include "cscenetabkeydeal.h"

/**
 * @brief The CWeekDaySceneTabKeyDeal class
 * 周/日场景tab切换
 */
class CWeekDaySceneTabKeyDeal : public CSceneTabKeyDeal
{
public:
    enum TabKeyType { ALLDayDeal,
                      PartTimeDeal }; //全体视图处理,非全天视图处理
public:
    explicit CWeekDaySceneTabKeyDeal(QGraphicsScene *scene = nullptr);

protected:
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CWEEKDAYSCENETABKEYDEAL_H
