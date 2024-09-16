// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CKEYENABLEDEAL_H
#define CKEYENABLEDEAL_H

#include "ckeypressdealbase.h"

#include <QDate>
#include <QWidget>

/**
 * @brief The CKeyEnableDeal class
 * 回车键处理
 */
class CKeyEnableDeal : public CKeyPressDealBase
{
public:
    explicit CKeyEnableDeal(QGraphicsScene *scene = nullptr);
protected:
    //焦点项处理
    bool focusItemDeal(CSceneBackgroundItem *item, CGraphicsScene *scene) override;
private:
    //创建日程
    static void createSchedule(const QDateTime &createDate, QWidget *parent);
};

#endif // CKEYENABLEDEAL_H
