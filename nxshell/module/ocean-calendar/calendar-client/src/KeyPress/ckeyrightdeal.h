// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CKEYRIGHTDEAL_H
#define CKEYRIGHTDEAL_H

#include "ckeypressdealbase.h"

/**
 * @brief The CKeyRightDeal class
 *  右方向键处理
 */
class CKeyRightDeal : public CKeyPressDealBase
{
public:
    explicit CKeyRightDeal(QGraphicsScene *scene = nullptr);

protected:
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CKEYRIGHTDEAL_H
