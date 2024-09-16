// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "datausagemodel.h"
#include "appdatadetaildialog.h"
#include "../src/widgets/defendertable.h"

#include <DButtonBox>
#include <DTipLabel>
#include <DCommandLinkButton>
#include <DBackgroundGroup>
#include <DFrame>
#include <DSpinner>

#include <QItemDelegate>
#include <QStandardItemModel>
#include <QTableView>

DWIDGET_USE_NAMESPACE
using namespace def::widgets;

#define FlowWidget_Debug 1

class FlowWidget;
class FlowItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    FlowItemDelegate(FlowWidget *widget = nullptr, QObject *parent = nullptr);
    virtual ~FlowItemDelegate() {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

private:
    FlowWidget *m_widget;
};

class NetworkProtectionModel;
class NetworkProtectionWorker;
class FlowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FlowWidget(DataUsageModel *model = nullptr, QWidget *parent = nullptr);
    ~FlowWidget();
    void setWaitingPage(bool isWaiting);
Q_SIGNALS:

public Q_SLOTS:
    // 异步获取所有进程网速数据
    void asyncGetAllAppFlowListOnTime();
    void onRecAllAppFlowList(const int timeRangeType, const DefenderProcInfoList &flowLst);

private:
    void SetFlowAttributes();
    void updateTable();
    void showAppFlowDetailDialog(DefenderProcInfo procInfo);

private:
    DataUsageModel *m_model;
    QVBoxLayout *m_layout;
    DTipLabel *m_totalUsageLabel;
    DPushButton *m_timeRangeSelectBtn;
    QTableView *m_flowtableview;
    QStandardItemModel *m_flowitemmodel;
    // 流量表格模块
    DefenderTable *m_flowTableWidget;
    FlowItemDelegate *m_flowitem;
    // 当前选择显示的流量时间范围
    TimeRangeType m_onDispTimeRangeType;
    // 接收到的流量时间跨度类型
    TimeRangeType m_recedTimeRangeType;
    // 等待界面
    DSpinner *m_waitSpinner;
    // 所有应用流量列表
    DefenderProcInfoList m_allAppFlowList;
};
DEF_NAMESPACE_END
DEF_NETPROTECTION_NAMESPACE_END
