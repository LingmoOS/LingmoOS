// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SIDEBARVIEW_H
#define SIDEBARVIEW_H

#include "sidebarcalendarwidget.h"
#include "sidebaritemwidget.h"
#include "accountmanager.h"
#include <DTreeWidget>
#include <QWidget>
#include <QTreeWidgetItem>
#include "sidebartreewidgetitemdelegate.h"

class SidebarView : public QWidget
{
    Q_OBJECT
public:
    explicit SidebarView(QWidget *parent = nullptr);

signals:
    void signalScheduleHide();
public slots:
    //账户更新事件
    void slotAccountUpdate();
    //日程类型更新事件
    void slotScheduleTypeUpdate();
    //帐户登出信号
    void signalLogout(DAccount::Type);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initView();
    void initConnection();
    //初始化数据
    void initData();
    //初始化列表展开状态
    void initExpandStatus();
    //初始化本地账户列表
    void initLocalAccountItem();
    //初始化union账户列表
    void initUnionAccountItem();

    //重置日程类型item
    void resetJobTypeChildItem(SidebarAccountItemWidget *parentItemWidget);
    //重置item位置
    void resetTreeItemPos(QTreeWidgetItem *item);

private:
    QTreeWidget *m_treeWidget = nullptr;        //树结构
    SidebarAccountItemWidget* m_localItemWidget = nullptr;
    SidebarAccountItemWidget* m_unionItemWidget = nullptr;

    SidebarCalendarWidget *m_calendarWidget = nullptr;  //小日历

    SideBarTreeWidgetItemDelegate *delegate = nullptr;
};

#endif // SIDEBARVIEW_H
