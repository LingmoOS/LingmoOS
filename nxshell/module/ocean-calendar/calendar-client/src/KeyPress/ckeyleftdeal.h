// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CKEYLEFTDEAL_H
#define CKEYLEFTDEAL_H

#include "ckeypressdealbase.h"

/**
 * @brief The CKeyLeftDeal class
 *  左方向键处理
 */
class CKeyLeftDeal : public CKeyPressDealBase
{
public:
    explicit CKeyLeftDeal(QGraphicsScene *scene = nullptr);

protected:
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CKEYLEFTDEAL_H
