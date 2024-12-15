// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef DMTREEVIEW_H
#define DMTREEVIEW_H

#include "dmtreeviewdelegate.h"
#include "dmdiskinfobox.h"
#include "partitionwidget.h"

#include <DTreeView>

#include <QStandardItemModel>
#include <QWidget>
#include <QModelIndex>
#include <QMouseEvent>
#include <QSortFilterProxyModel>

DWIDGET_USE_NAMESPACE

/**
 * @class DmTreeview
 * @brief 树结构类
*/

class DmTreeview : public DTreeView
{
    Q_OBJECT
public:
    explicit DmTreeview(QWidget *parent = nullptr);

    /**
     * @brief 排序
    */
    void sort();

    /**
     * @brief 添加子节点
     * @param item 节点指针
     * @param data 节点数据
     * @param flag 是否展开根节点标志
    */
    QStandardItem * addItem(QStandardItem *item, const DiskInfoData &data, int flag);

    /**
     * @brief 获取指向与给定索引关联的QStandardItem的指针
     * @param index 索引值
     * @return 返回QStandardItem指针
    */
    QStandardItem *getItemByIndex(const QModelIndex &index);

    /**
     * @brief 添加根节点
     * @param data 节点数据
     * @return 返回QStandardItem指针
    */
    QStandardItem *addTopItem(const DiskInfoData &data);

    /**
     * @brief 获取当前节点
     * @return 返回QStandardItem指针
    */
    QStandardItem *getCurItem();

    /**
     * @brief 获取根节点
     * @return 返回QStandardItem指针
    */
    QStandardItem *getRootItem();

    /**
     * @brief 获取根节点索引值
     * @return 返回索引值
    */
    QModelIndex getRootItemIndex();

    /**
     * @brief 设置默认选中节点
     * @return 返回节点索引值
    */
    QModelIndex setDefaultdmItem();

    /**
     * @brief 设置刷新后默认选择操作分区
     * @param devicenum 设备树索引
     * @param num 设备树子节点索引
    */
    void setRefreshItem(int devicenum, int num);

    /**
     * @brief 设置刷新后默认选择操作分区
     * @param devicenum 设备树索引
     * @param num 设备树子节点索引
     * @param groupNum 组索引（设备组或逻辑卷组）
    */
    void setRefreshItem(int devicenum, int num, int groupNum);

    /**
     * @brief 获取当前选中分区索引值
     * @return 当前选中分区索引值
    */
    int getCurrentNum();

    /**
     * @brief 获取当前选中分区的父节点索引值
     * @return 当前选中分区父节点索引值
    */
    int getCurrentTopNum();

    /**
     * @brief 获取当前选中分区的组节点索引值
     * @return 当前选中分区组节点索引值
    */
    int getCurrentGroupNum();

    /**
     * @brief 添加子节点
     * @param infoBox 节点数据
     * @param flag 是否展开根节点标志
     * @param purItem 节点指针
    */
    void addItem(DmDiskinfoBox *infoBox, int flag, QStandardItem *purItem = nullptr);

    /**
     * @brief 添加根节点
     * @param mailbox 节点数据
     * @param flag 是否展开根节点标志
    */
    void addTopItem(DmDiskinfoBox *mailBox, int flag);

    /**
     * @brief 添加节点
     * @param mailbox 节点数据
     * @param curItem 节点指针
     * @param flag 是否展开根节点标志
    */
    void addSubItem(DmDiskinfoBox *mailBox, QStandardItem *curItem, int flag);

    /**
     * @brief 清除根节点数据
    */
    void clearData();

    QStandardItemModel *m_model {nullptr};

signals:
    void selectItem(const QModelIndex &index);
    void curSelectChanged(const QString &devicepath, const QString &partitionpath, Sector start, Sector end, int level);
    //    void sigSendInfo(const QString &devicepath, const QString &disksize, const QString &partitionpath, const QString &partsize, const QString &style, Sector start, Sector end);

public slots:

private:
    /**
     * @brief 初始化界面
    */
    void initUI();

    /**
     * @brief 初始化模型
    */
    void initModel();

    /**
     * @brief 初始化Delegate
    */
    void initDelegate();

private:
    QAbstractItemDelegate *m_delegate {nullptr};
    QSortFilterProxyModel *m_sortViewFilter {nullptr};
    QString m_diskSize;
    int m_curNum = 0;
    int m_diskNum = 0;
    int m_groupNum = 0;
    bool isGroup = false;
    QList<QStandardItem *> m_lstStandardItem;

protected:

    /**
     * @brief 重写当前索引改变函数
    */
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    /**
     * @brief 重写点击事件
    */
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // DMTREEVIEW_H
