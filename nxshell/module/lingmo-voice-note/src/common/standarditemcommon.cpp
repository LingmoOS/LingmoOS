// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "standarditemcommon.h"

/**
 * @brief StandardItemCommon::StandardItemCommon
 */
StandardItemCommon::StandardItemCommon()
{
}

/**
 * @brief StandardItemCommon::createStandardItem
 * @param data 绑定的数据
 * @param type
 * @return 生成数据项
 */
QStandardItem *StandardItemCommon::createStandardItem(void *data, StandardItemType type)
{
    QStandardItem *pItem = new QStandardItem;
    pItem->setData(QVariant::fromValue(type), Qt::UserRole + 1);
    pItem->setData(QVariant::fromValue(data), Qt::UserRole + 2);
    return pItem;
}

/**
 * @brief StandardItemCommon::getStandardItemType
 * @param index 索引
 * @return 数据项类型
 */
StandardItemCommon::StandardItemType StandardItemCommon::getStandardItemType(const QModelIndex &index)
{
    StandardItemType type = Invalid;
    if (index.isValid()) {
        QVariant var = index.data(Qt::UserRole + 1);
        if (var.isValid()) {
            return var.value<StandardItemType>();
        }
    }
    return type;
}

/**
 * @brief StandardItemCommon::getStandardItemData
 * @param index
 * @return 数据项绑定的数据
 */
void *StandardItemCommon::getStandardItemData(const QModelIndex &index)
{
    if (index.isValid()) {
        QVariant var = index.data(Qt::UserRole + 2);
        if (var.isValid()) {
            return var.value<void *>();
        }
    }
    return nullptr;
}
