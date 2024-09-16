// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "speedwidget.h"
#include "flowwidget.h"

#include <DButtonBox>
#include <DTipLabel>
#include <DCommandLinkButton>
#include <DBackgroundGroup>
#include <DFrame>
#include <DPushButton>

#include <QItemDelegate>
#include <QStandardItemModel>

enum TabelType {
    AppsServices,
    Trafficranking,
};

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QTableView;
class QPushButton;
class QStandardItemModel;
class QStandardItem;
class QComboBox;
class QToolButton;
QT_END_NAMESPACE
DEF_NAMESPACE_BEGIN
DEF_NETPROTECTION_NAMESPACE_BEGIN
class DataUsageModel;
class DataUsageWidget : public DFrame
{
    Q_OBJECT
public:
    explicit DataUsageWidget(DataUsageModel *model = nullptr, QWidget *parent = nullptr);
    ~DataUsageWidget();

public Q_SLOTS:

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void initUI();
    void showAppsServices();
    void showTrafficranking();
    void settype(TabelType type);

private:
    DataUsageModel *m_model;
    SpeedWidget *m_speedwidget;
    FlowWidget *m_flowwidget;
    QVBoxLayout *m_layout; //最大布局
    QWidget *m_widget;
    DBackgroundGroup *m_bgGroup;
    Dtk::Widget::DButtonBox *m_topSwitchWidgetBtn;
    QList<Dtk::Widget::DButtonBoxButton *> m_btnlist;
    bool m_tabeltype = 0;
    QVBoxLayout *m_tablelayout;
};
DEF_NAMESPACE_END
DEF_NETPROTECTION_NAMESPACE_END
