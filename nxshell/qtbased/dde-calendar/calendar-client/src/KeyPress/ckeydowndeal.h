// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CKEYDOWNDEAL_H
#define CKEYDOWNDEAL_H

#include "ckeypressdealbase.h"

/**
 * @brief The CKeyDownDeal class
 *  下方向键处理
 */
class CKeyDownDeal : public CKeyPressDealBase
{
public:
    explicit CKeyDownDeal(QGraphicsScene *scene = nullptr);

protected:
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
};

#endif // CKEYDOWNDEAL_H
