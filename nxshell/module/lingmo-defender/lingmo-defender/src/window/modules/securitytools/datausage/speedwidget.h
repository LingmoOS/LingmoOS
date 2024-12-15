// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "datausagemodel.h"
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
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace def::widgets;

#define SpeedWidget_Debug 1

class SpeedWidget;
class SpeedItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    SpeedItemDelegate(SpeedWidget *widget, QObject *parent = nullptr);
    virtual ~SpeedItemDelegate() {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

private:
    SpeedWidget *m_widget;
};

class TipWidget;
class NetworkProtectionModel;
class NetworkProtectionWorker;
class SpeedWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpeedWidget(DataUsageModel *model = nullptr, QWidget *parent = nullptr);
    ~SpeedWidget();

private:
    void SetSpeedAttributes();

    // 初始化判断是否显示联网管控开关未开启提示
    void intiSwitchStatus();

private Q_SLOTS:
    // 表格内下拉框选项改变
    void onAppOptChanged(int index);

public Q_SLOTS:
    // 更新表格数据
    void updateTable();
    // 当收到所有进程信息列表时
    void recAllProcInfos(const DefenderProcInfoList &procInfos);

private:
    DataUsageModel *m_model;
    QVBoxLayout *m_layout;
    DTipLabel *m_totalSpeedLabel;
    QTableView *m_speedtableview;
    QStandardItemModel *m_speeditemmodel;

    // 应用网速表格模块
    DefenderTable *m_speedTableWidget;
    SpeedItemDelegate *m_speeditem;
    QMap<QComboBox *, QString> m_comboToAppName;
    QMap<QString, QString> m_appNamefromPkgName;

    DWidget *m_tipwidSwitchOff;
    TipWidget *m_tipwidgetSwitchOff;

    //等待界面
    DSpinner *m_waitSpinner;
    DefenderProcInfoList m_mergedProcInfos;
};
DEF_NAMESPACE_END
DEF_NETPROTECTION_NAMESPACE_END
