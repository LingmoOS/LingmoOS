// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "cscenebackgrounditem.h"

#include <QGraphicsScene>
#include <QDebug>
#include <QMarginsF>

CSceneBackgroundItem::CSceneBackgroundItem(ItemOnView view, QGraphicsItem *parent)
    : CFocusItem(parent)
    , m_backgroundNum(0)
    , m_leftItem(nullptr)
    , m_rightItem(nullptr)
    , m_upItem(nullptr)
    , m_downItem(nullptr)
    , m_showItemIndex(-1)
    , m_itemOfView(view)
{
    //设置item类型为背景显示
    setItemType(CBACK);
}

/**
 * @brief CSceneBackgroundItem::setNextItemFocusAndGetNextItem  设置下一个item focus状态并获取下一个Item
 * @return
 */
CFocusItem *CSceneBackgroundItem::setNextItemFocusAndGetNextItem()
{
    CFocusItem *NextFocus = this;
    //若该区域没有item
    if (m_showItemIndex < 0 && m_item.size() == 0) {
        NextFocus = CFocusItem::setNextItemFocusAndGetNextItem();
    } else if (m_showItemIndex == m_item.size() - 1) {
        //若切换到最后一个item
        m_item.at(m_showItemIndex)->setItemFocus(false);
        m_showItemIndex = -1;
        NextFocus = CFocusItem::setNextItemFocusAndGetNextItem();
    } else {
        //若该背景上有显示的item
        //若显示的item未设置focus则取消背景focus效果
        if (m_showItemIndex == -1 && getItemFocus()) {
            this->setItemFocus(false);
        }
        //若显示的item有设置focus则取消该item focus效果
        if (m_showItemIndex >= 0) {
            m_item.at(m_showItemIndex)->setItemFocus(false);
        }
        //当前显示的item编号+1并这是focus效果
        ++m_showItemIndex;
        m_item.at(m_showItemIndex)->setItemFocus(true);
    }
    return NextFocus;
}

/**
 * @brief compareItemData       对现实的日程标签进行排序
 * @param itemfirst
 * @param itemsecond
 * @return
 */
bool compareItemData(const CFocusItem *itemfirst, const CFocusItem *itemsecond)
{
    if (itemfirst->rect() == itemsecond->rect()) {
        return false;
    }
    //根据从上倒下从左至右的规则对矩阵的x,y坐标进行对比排序
    if (qAbs(itemfirst->rect().y() - itemsecond->rect().y()) < 0.01) {
        if (itemfirst->rect().x() < itemsecond->rect().x()) {
            return true;
        } else {
            return false;
        }
    } else if (itemfirst->rect().y() < itemsecond->rect().y()) {
        return true;
    } else {
        return false;
    }
}
/**
 * @brief CSceneBackgroundItem::updateShowItem  更新在此背景上显示的item
 */
void CSceneBackgroundItem::updateShowItem()
{
    m_item.clear();
    //缩小背景矩阵,防止获取到其他背景上的item
    QRectF offsetRect = this->rect().marginsRemoved(QMarginsF(1, 1, 1, 1));
    QList<QGraphicsItem *> mlistitem = this->scene()->items(offsetRect);
    for (int i = 0; i < mlistitem.count(); ++i) {
        CFocusItem *item = dynamic_cast<CFocusItem *>(mlistitem.at(i));
        if (item != nullptr && item->getItemType() != CBACK) {
            m_item.append(item);
        }
    }
    qSort(m_item.begin(), m_item.end(), compareItemData);
    updateCurrentItemShow();
}

int CSceneBackgroundItem::getShowItemCount()
{
    return m_item.size();
}

/**
 * @brief CSceneBackgroundItem::setBackgroundNum        设置背景编号
 * @param num
 */
void CSceneBackgroundItem::setBackgroundNum(int num)
{
    m_backgroundNum = num;
}

/**
 * @brief CSceneBackgroundItem::getBackgroundNum        获取背景编号
 * @return
 */
int CSceneBackgroundItem::getBackgroundNum() const
{
    return m_backgroundNum;
}

/**
 * @brief CSceneBackgroundItem::setItemFocus 设置item是否获取focus
 * @param isFocus
 */
void CSceneBackgroundItem::setItemFocus(bool isFocus)
{
    if (m_showItemIndex < 0) {
        CFocusItem::setItemFocus(isFocus);
    } else {
        if (m_showItemIndex < m_item.size()) {
            m_item.at(m_showItemIndex)->setItemFocus(isFocus);
        }
    }
}

/**
 * @brief CSceneBackgroundItem::initState   恢复初始状态
 */
void CSceneBackgroundItem::initState()
{
    if (getItemFocus()) {
        setItemFocus(false);
    }
    if (m_showItemIndex > -1 && m_showItemIndex < m_item.size()) {
        m_item.at(m_showItemIndex)->setItemFocus(false);
    }
    m_showItemIndex = -1;
}

/**
 * @brief CSceneBackgroundItem::getFocusItem    获取当前焦点的item
 * @return
 */
CFocusItem *CSceneBackgroundItem::getFocusItem()
{
    if (m_showItemIndex < 0) {
        return this;
    } else {
        return m_item.at(m_showItemIndex);
    }
}

CSceneBackgroundItem *CSceneBackgroundItem::getLeftItem() const
{
    return m_leftItem;
}

void CSceneBackgroundItem::setLeftItem(CSceneBackgroundItem *leftItem)
{
    m_leftItem = leftItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getRightItem() const
{
    return m_rightItem;
}

void CSceneBackgroundItem::setRightItem(CSceneBackgroundItem *rightItem)
{
    m_rightItem = rightItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getUpItem() const
{
    return m_upItem;
}

void CSceneBackgroundItem::setUpItem(CSceneBackgroundItem *upItem)
{
    m_upItem = upItem;
}

CSceneBackgroundItem *CSceneBackgroundItem::getDownItem() const
{
    return m_downItem;
}

void CSceneBackgroundItem::setDownItem(CSceneBackgroundItem *downItem)
{
    m_downItem = downItem;
}

void CSceneBackgroundItem::updateCurrentItemShow()
{
    if (m_showItemIndex >= 0) {
        if (m_item.size() > 0) {
            m_showItemIndex = m_showItemIndex < m_item.size() ? m_showItemIndex : 0;
            m_item.at(m_showItemIndex)->setItemFocus(true);
        } else {
            m_showItemIndex = -1;
            setItemFocus(true);
        }
    }
}

CSceneBackgroundItem::ItemOnView CSceneBackgroundItem::getItemOfView() const
{
    return m_itemOfView;
}
