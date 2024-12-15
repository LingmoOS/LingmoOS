// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKAGESLISTDELEGATE_H
#define PACKAGESLISTDELEGATE_H

#include "packagelistview.h"

#include <DStyledItemDelegate>
#include <QSettings>

class AbstractPackageListModel;

class PackagesListDelegate : public DStyledItemDelegate
{
    Q_OBJECT

public:
    // delegate使用传入的model而非重新new一个对象 解决多次创建model packagemanager导致崩溃的问题
    explicit PackagesListDelegate(AbstractPackageListModel *m_model, QAbstractItemView *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event) override;

public:
    /**
     * @brief getItemHeight 获取当前选中的Item的高度
     * @param height 当前选中Item的高度
     *
     */
    void getItemHeight(int height);

signals:
    /**
     * @brief sigIndexAndRect 发送当前Item的位置参数和row。确定右键菜单的位置。
     * @param rect 当前rect的位置等参数
     * @param index 当前item的下标
     */
    void sigIndexAndRect(QRect rect, int index) const;

private:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    /**
     * @brief refreshDebItemStatus 刷新每个item 包的状态
     * @param operate_stat         操作状态
     * @param install_status_rect  安装状态的位置
     * @param painter
     * @param isSelect              是否被选中
     * @param isEnable              是否可用
     */
    void refreshDebItemStatus(
        const int operate_stat, QRect install_status_rect, QPainter *painter, bool isSelect, bool isEnable) const;

private:
    QPixmap m_packageIcon;  // 包的图标
    QSettings m_qsettings;  // 废弃变量

    // item的高度
    int m_itemHeight = 0;

    // 传入的model
    AbstractPackageListModel *m_fileListModel = nullptr;

    QAbstractItemView *m_parentView;
};

#endif  // PACKAGESLISTDELEGATE_H
