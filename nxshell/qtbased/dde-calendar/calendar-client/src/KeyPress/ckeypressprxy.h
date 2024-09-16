// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CKEYPRESSPRXY_H
#define CKEYPRESSPRXY_H

#include "ckeypressdealbase.h"

#include <QMap>
#include <QKeyEvent>

class CKeyPressPrxy
{
public:
    explicit CKeyPressPrxy();
    ~CKeyPressPrxy();
    //键盘事件处理
    bool keyPressDeal(int key);
    //添加需要处理的键盘事件
    void addkeyPressDeal(CKeyPressDealBase *deal);
    //移除添加的键盘事件
    void removeDeal(CKeyPressDealBase *deal);

private:
    QMap<int, CKeyPressDealBase *> m_keyEventMap {};
};

#endif // CKEYPRESSPRXY_H
