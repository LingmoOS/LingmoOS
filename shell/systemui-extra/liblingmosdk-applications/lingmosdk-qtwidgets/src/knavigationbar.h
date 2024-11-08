/*
 * liblingmosdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KNAVIGATIONBAR_H
#define KNAVIGATIONBAR_H

#include "gui_g.h"
#include <QScrollArea>
#include <QStandardItem>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QListView>
#include <QMouseEvent>

namespace kdk
{

/** @defgroup BarModule
  * @{
  */

class KNavigationBarPrivate;
class ListView;

/**
 * @brief 导航栏控件
 */
class GUI_EXPORT KNavigationBar:public QScrollArea
{
    Q_OBJECT

public:
    KNavigationBar(QWidget*parent);

    /**
     * @brief 增加常规Item
     * @param item
     */
    void addItem(QStandardItem*item);

    /**
     * @brief 增加次级Item
     * @param subItem
     */
    void addSubItem(QStandardItem*subItem);

    /**
     * @brief 成组增加Item,在导航栏中会显示tag
     * @param items
     * @param tag
     */
    void addGroupItems(QList<QStandardItem*>items,const QString& tag);

    /**
     * @brief 添加tag
     * @param tag
     */
    void addTag(const QString& tag);

    /**
     * @brief 获取model
     * @return
     */
    QStandardItemModel* model();

    /**
     * @brief 获取view
     * @return
     */
    QListView* listview();

private:
    Q_DECLARE_PRIVATE(KNavigationBar)
    KNavigationBarPrivate*const d_ptr;
};

}
/**
  * @example testnavigationbar/widget.h
  * @example testnavigationbar/widget.cpp
  * @}
  */
#endif // KNAVIGATIONBAR_H
