// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file tableModel.h
 *
 * @brief 表格数据管理模块
 *
 * @date 2020-06-09 09:58
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TABLEVIEWMODEL_H
#define TABLEVIEWMODEL_H

#include <QAbstractTableModel>

namespace Global {
struct DownloadDataItem;
struct DeleteDataItem;
} // namespace Global

/**
 * @class TableModel
 * @brief 表格数据管理模块
 */
class TableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    /**
     * @brief 列表模式
     */
    enum DownloadMode {
        Downloading = 0,
        Finished
    };
    /**
     * @brief 数据角色
     */
    enum DataRole {
        Ischecked = 1000,
        Speed,
        Size,
        FileName,
        Time,
        Status,
        GID,
        Percent,
        TotalLength,
        SavePath,
        taskId,
        createTime,
        IsHide,
        connection,
        announceList
    };

    /**
     * @brief 构造函数
     */
    TableModel(int Flag, QObject *parent = nullptr);

    /**
     * @brief 父类虚函数，设置数据
     * @param ndex: 索引
     * @param value: 数据
     * @param role: 角色
     */
    bool setData(const QModelIndex &ndex,
                 const QVariant &value,
                 int role) override;

    /**
     * @brief 在正在下载列表添加一列
     * @param data: 数据
     */
    bool append(Global::DownloadDataItem *data);

    /**
     * @brief 排序
     * @param column: 列
     * @param order: 排序方式
     */
    virtual void sort(int column, Qt::SortOrder order) override;

    /**
     * @brief 在回收站列表添加一列
     * @param data: 数据
     */
    bool append(Global::DeleteDataItem *data);

    /**
     * @brief 在正在下载列表删除一列
     * @param data: 数据
     */
    bool removeItem(Global::DownloadDataItem *data);

    /**
     * @brief 在回收站列表删除一列
     * @param data: 数据
     */
    bool removeItem(Global::DeleteDataItem *data);

    /**
     * @brief 清空下载列表
     */
    bool removeItems();

    /**
     * @brief 清空回收站列表
     */
    bool removeRecycleItems();

    /**
     * @brief 切换到正在下载列表
     */
    bool switchDownloadingMode();

    /**
     * @brief 切换到已完成列表
     */
    bool switchFinishedMode();

    /**
     * @brief 根据id查找正在下载item
     * @param taskId: 任务id
     * @return 查找到的item数据
     */
    Global::DownloadDataItem *find(const QString &taskId);

    /**
     * @brief 根据id查找回收站item
     * @param gid: id
     * @return 查找到的item数据
     */
    Global::DeleteDataItem *find(const QString &gid, int flag);

    /**
     * @brief 获取正在下载列表
     * @return item列表
     */
    const QList<Global::DownloadDataItem *> &dataList();

    /**
     * @brief 获取已完成下载列表
     * @return item列表
     */
    const QList<Global::DownloadDataItem *> &renderList();

    /**
     * @brief 获取回收站下载列表
     * @return item列表
     */
    const QList<Global::DeleteDataItem *> &recyleList();

    /**
     * @brief 获取列表展示模式
     * @return 模式
     */
    int getTablemodelMode();

    /**
     * @brief 获取列表展示模式
     * @return 模式map
     */
    QMap<QString, Global::DownloadDataItem *> &getTableModelMap();

    /**
     * @brief 获取正在下载的个数
     * @return 正在下载的个数
     */
    int DownloadingCount();

    /**
     * @brief 获取行数
     * @param index: 索引
     * @return 行数
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief 获取列数
     * @param index: 索引
     * @return 列数
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief 获取数据
     * @param index: 索引
     * @param role: 角色
     * @return 数据
     */
    QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief 获取表头
     * @param role: 角色
     * @return 表头数据
     */
    QVariant headerData(int ection, Qt::Orientation orientation, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    ~TableModel();

private:
    void sortDownload(int column, Qt::SortOrder order);
    void sortRecycle(int column, Qt::SortOrder order);
signals:
    /**
     * @brief 改变选中
     * @param flag: 模式
     */
    void checkDatachange(int flag);
    /**
     * @brief 全选或者全不选
     * @param checked: 选中状态
     */
    void tableviewAllcheckedOrAllunchecked(bool checked);
    /**
     * @brief 选中或者不选中
     * @param flag: 模式
     */
    void CheckChange(bool checked, int flag);

    /**
     * @brief 排序后数据改变
     */
    void layoutChanged();
private slots:
    /**
     * @brief 获取选中改变
     * @param flag: 模式
     */
    void onCheckdatachange(int flag);

private:
    QList<Global::DownloadDataItem *> m_DataList;
    QList<Global::DownloadDataItem *> m_RenderList; //切换到正在下载列表就保存正在下载的数据，切换到已完成列表就保存已完成数据
    QMap<QString, Global::DownloadDataItem *> m_Map;
    DownloadMode m_Mode;
    int m_TableviewtabFlag;
    int m_DownloadingCount;
    QMap<QString, Global::DeleteDataItem *> m_Deletemap;
    QList<Global::DeleteDataItem *> m_RecyleList;
    int m_SortColumn;
    Qt::SortOrder m_SortOrder;
};

#endif // TABLEVIEWMODEL_H
