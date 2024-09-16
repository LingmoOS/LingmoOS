// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ckeypressprxy.h"

#include <QDebug>

CKeyPressPrxy::CKeyPressPrxy()
{
}

CKeyPressPrxy::~CKeyPressPrxy()
{
    QMap<int, CKeyPressDealBase *>::iterator iter = m_keyEventMap.begin();
    for (; iter != m_keyEventMap.end(); ++iter) {
        delete iter.value();
    }
    m_keyEventMap.clear();
}

/**
 * @brief CKeyPressPrxy::keyPressDeal   键盘事件处理
 * @param event
 * @return
 */
bool CKeyPressPrxy::keyPressDeal(int key)
{
    bool result = m_keyEventMap.contains(key);
    if (result) {
        //如果有注册对应的key事件 开始处理
        result = m_keyEventMap[key]->dealEvent();
    }
    return result;
}

/**
 * @brief CKeyPressPrxy::addkeyPressDeal    添加需要处理的键盘事件
 * @param deal
 */
void CKeyPressPrxy::addkeyPressDeal(CKeyPressDealBase *deal)
{
    if (deal != nullptr)
        m_keyEventMap[deal->getKey()] = deal;
}

/**
 * @brief CKeyPressPrxy::removeDeal     移除添加的键盘事件
 * @param deal
 */
void CKeyPressPrxy::removeDeal(CKeyPressDealBase *deal)
{
    if (m_keyEventMap.contains(deal->getKey())) {
        m_keyEventMap.remove(deal->getKey());
        delete deal;
    }
}
