// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HEADERTABLEVIEW_H
#define HEADERTABLEVIEW_H

#include <DTableView>
#include <DStandardItem>
#include <DHeaderView>

#include <QStandardItemModel>
#include <QObject>
#include <QHBoxLayout>

class LogTreeView;

using namespace Dtk::Widget;

/**
 * @brief The TableWidget class
 * UI : 自定义tablewidget
 */
class TableWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TableWidget(QWidget *parent = nullptr);
    ~TableWidget() override;

    /**
     * @brief setHeaderLabels : 设置table的表头
     * @param lst ：表头的内容
     */
    void setHeaderLabels(const QStringList &lst);

    /**
     * @brief setItem : 设置表格的item
     * @param row : item设置到哪一行
     * @param column : item设置到哪一列
     * @param item ：需要设置的item
     */
    void setItem(int row, int column, DStandardItem *item);

    /**
     * @brief setColumnAverage
     */
    void setColumnAverage();

    /**
     * @brief updateCurItemEnable
     * @param row
     * @param enable
     */
    void updateCurItemEnable(int row, bool enable);

    /**
     * @brief clear : 清空数据
     */
    void clear();

    /**
     * @brief setRowNum: 设置表格行数
     * @param row : 总行数
     */
    void setRowNum(int row);

signals:
    void itemClicked(int row);
    void refreshInfo();
    void exportInfo();
    void enableDevice(int row, bool enable);
    void uninstallDriver(int row);
    void installDriver(int row);
    void wakeupMachine(int row, bool wakeup);
    void signalCheckPrinterStatus(int row, bool &isPrinter, bool &isInstalled);
protected:
    void paintEvent(QPaintEvent *e) override;

private slots:

    /**
     * @brief slotShowMenu:鼠标右键菜单槽函数
     */
    void slotShowMenu(const QPoint &p);

    /**
     * @brief slotActionRefresh:右键刷新按钮槽函数
     */
    void slotActionRefresh();

    /**
     * @brief slotActionExport:右键导出按钮槽函数
     */
    void slotActionExport();

    /**
     * @brief slotActionEnable:判断是否启用
     */
    void slotActionEnable();

    /**
     * @brief slotActionCopy:右键菜单更新驱动
     */
    void slotActionUpdateDriver();

    /**
     * @brief slotActionCopy:右键菜单卸载驱动
     */
    void slotActionRemoveDriver();

    /**
     * @brief slotWakeupMachine:设置可以唤醒机器
     */
    void slotWakeupMachine();

    /**
     * @brief slotItemClicked:ListView Item 点击槽函数
     * @param index:点击Item的索引
     */
    void slotItemClicked(const QModelIndex &index);

private:
    void initWidget();

private:
    LogTreeView      *mp_Table;
    QHBoxLayout      *m_HLayout;
    QAction          *mp_Enable;      //<! 启用禁用
    QAction          *mp_Refresh;     //<! 右键刷新
    QAction          *mp_Export;      //<! 右键导出
    QAction          *mp_updateDriver;//<! 驱动更新
    QAction          *mp_removeDriver;//<! 驱动卸载
    QAction          *mp_WakeupMachine;//<! 唤醒机器
    QMenu            *mp_Menu;        //<! 右键菜单
    bool             m_Enable;
};






#endif // HEADERTABLEVIEW_H
