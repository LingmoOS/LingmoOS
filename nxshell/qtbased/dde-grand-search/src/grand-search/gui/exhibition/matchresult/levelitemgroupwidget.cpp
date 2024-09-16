// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "levelitemgroupwidget.h"
#include "listview/grandsearchlistview.h"
#include "viewmore/viewmorebutton.h"
#include "utils/utils.h"

GRANDSEARCH_USE_NAMESPACE

LevelItemGroupWidget::LevelItemGroupWidget(QWidget *parent)
    : GroupWidget (parent)
{

}

void LevelItemGroupWidget::appendMatchedItems(const GrandSearch::MatchedItems &newItems, const QString &searchGroupName)
{
    Q_UNUSED(searchGroupName)

    if (Q_UNLIKELY(newItems.isEmpty()))
        return;

    // 数据拆分
    QMap<int, GrandSearch::MatchedItems> newLevelItems;
    MatchedItems newGeneralItems;
    splitLevelData(newItems, newLevelItems, newGeneralItems);

    // 结果列表未展开
    if (!m_bListExpanded) {

        // 新来数据先放入对应层级的缓存中
        for (auto level : newLevelItems.keys()) {
            m_levelCacheItems[level].append(newLevelItems.value(level));
        }
        m_cacheItems << newGeneralItems;

        // 显示不足5个，或显示的5个中存在不是分层项，而现在又有新的分层项时，分层项需要添加显示
        if (GROUP_MAX_SHOW != m_listView->rowCount()
                || (GROUP_MAX_SHOW != m_listView->levelItemCount() && !newLevelItems.isEmpty())) {

            // 清空列表数据，将已显示数据还原到各自缓存中
            for (auto item : m_firstFiveItems) {
                int tempLevel = -1;
                if (Utils::isLevelItem(item, tempLevel))
                    m_levelCacheItems[tempLevel].append(item);
                else
                    m_cacheItems.append(item);
            }
            m_firstFiveItems.clear();

            takeItemFromLevelCache();

            takeItemFromGeneralCache();
        }

        // 缓存中有数据，显示'查看更多'按钮
        m_viewMoreButton->setVisible(!m_cacheItems.isEmpty() || (!m_levelCacheItems.isEmpty() && !m_levelCacheItems.first().isEmpty()));
    } else {
        // 结果列表已展开
        // 对新数据排序，并插入到已显示结果末尾
        for (auto level : newLevelItems.keys()) {
            Utils::sort(newLevelItems[level]);
        }

        for (auto level : newLevelItems.keys()) {
            auto items = newLevelItems.value(level);
            m_listView->addRows(items, level);
        }

        Utils::sort(newGeneralItems);
        m_listView->addRows(newGeneralItems);
    }

    layout();
}

void LevelItemGroupWidget::clear()
{
    m_levelCacheItems.clear();

    GroupWidget::clear();
}

void LevelItemGroupWidget::onMoreBtnClicked()
{
    Q_ASSERT(m_listView);

    for (auto level : m_levelCacheItems.keys()) {
        Utils::sort(m_levelCacheItems[level]);
    }

    // 在已显示分层项的对应层级的最后一行，增加缓存中的分层项
    for (auto level : m_levelCacheItems.keys()) {
        auto items = m_levelCacheItems.value(level);
        m_listView->addRows(items, level);

        items.clear();
    }
    m_levelCacheItems.clear();

    GroupWidget::onMoreBtnClicked();
}

void LevelItemGroupWidget::takeItemFromLevelCache()
{
    // 拉通重排缓存中各个层级的匹配结果
    for (auto level : m_levelCacheItems.keys()) {
        Utils::sort(m_levelCacheItems[level]);
    }

    while (m_firstFiveItems.count() < GROUP_MAX_SHOW) {

        // 如果存在分层项，则依次取各层数据项使用
        if (!m_levelCacheItems.isEmpty()) {
            GrandSearch::MatchedItems &items = m_levelCacheItems.first();

            if (items.isEmpty()) {
                // 如果该层的数据项为空，则移除该层,并继续取下一层的数据
                int key = m_levelCacheItems.firstKey();
                m_levelCacheItems.remove(key);
                continue;
            }

            for (int i=m_firstFiveItems.count(); i<GROUP_MAX_SHOW; i++) {
                GrandSearch::MatchedItem item = items.takeFirst();
                m_firstFiveItems.push_back(item);

                if (items.isEmpty()) {
                    // 如果该层的数据项已经取完，则移除该层,并继续取下一层的数据
                    int key = m_levelCacheItems.firstKey();
                    m_levelCacheItems.remove(key);
                    break;
                }
            }
        }
        // 所有层中的数据取完后仍然不够时，退出循环，并从普通缓存中继续取数据
        break;
    }

    m_listView->setMatchedItems(m_firstFiveItems);
}

void LevelItemGroupWidget::takeItemFromGeneralCache()
{
    // 层级项不足5个，从一般缓存中取剩余数据补齐5个
    if (m_firstFiveItems.size() < GROUP_MAX_SHOW) {
        Utils::sort(m_cacheItems);

        for (int i = m_firstFiveItems.size(); i < GROUP_MAX_SHOW; i++) {
            if (!m_cacheItems.isEmpty()) {
                MatchedItem item = m_cacheItems.takeFirst();
                m_firstFiveItems.push_back(item);

                m_listView->addRow(item);
            }
        }
    }
}

void LevelItemGroupWidget::splitLevelData(const GrandSearch::MatchedItems &newItems, QMap<int, MatchedItems> &newLevelItems, MatchedItems &newGeneralItems)
{
    for (auto item : newItems) {
        int tempLevel = -1;
        if (Utils::isLevelItem(item, tempLevel)) {
            newLevelItems[tempLevel].append(item);
        } else {
            // 理论上，从后端获取的数据中，如果存在分层项，则必然是从第0个开始且连续
            // 即，只要判断到一个不属于分层项，则该组数据之后的项都不属于分层项
            // 如果全量判断存在显示效率，则考虑优化此处的判断方式
            newGeneralItems << item;
        }
    }
}
