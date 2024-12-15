// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LEVELITEMGROUPWIDGET_H
#define LEVELITEMGROUPWIDGET_H

#include "groupwidget.h"

namespace GrandSearch {

class LevelItemGroupWidget : public GroupWidget
{
public:
    explicit LevelItemGroupWidget(QWidget *parent = nullptr);

    void appendMatchedItems(const MatchedItems &newItems, const QString& searchGroupName) override;
    void clear() override;

public slots:
    void onMoreBtnClicked() override;

private:
    void takeItemFromLevelCache();
    void takeItemFromGeneralCache();
    void splitLevelData(const MatchedItems &newItems, QMap<int, MatchedItems> &newLevelItems, MatchedItems &newGeneralItems);
private:
    QMap<int, MatchedItems> m_levelCacheItems;     //<层级, 该层的缓解项>
};

}

#endif // LEVELITEMGROUPWIDGET_H
