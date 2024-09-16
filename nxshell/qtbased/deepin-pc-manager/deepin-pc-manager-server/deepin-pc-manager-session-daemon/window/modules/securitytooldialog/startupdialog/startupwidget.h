// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "widgets/defendertable.h"
#include "../defsecuritytoolsmnd.h"

#include <QItemDelegate>

class DBusInvokerInterface;
class AutoStartModel;

DWIDGET_USE_NAMESPACE
using namespace def::widgets;

/************************* StartupWidget实现类 表格代理 ***********************/
class StartupWidget;
class MyItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    MyItemDelegate(StartupWidget *widget, QObject *parent = nullptr);
    virtual ~MyItemDelegate() {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

private:
    QPixmap m_pixmap;
    StartupWidget *m_widget;
};

/************************* StartupWidget实现类 表格颜色重载 ***********************/
class StartupModel : public QStandardItemModel
{
    Q_OBJECT
public:
    StartupModel();
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

/**************************** StartupWidget实现类 ********************************/
class QLabel;
class StartupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StartupWidget(AutoStartModel *model, QWidget *parent = nullptr);
    ~StartupWidget();

    QPixmap getPixmap(QPixmap);
Q_SIGNALS:
    void requstCloseInfo();

public Q_SLOTS:
    // 表格按钮点击槽
    void onTableBtnClicked();
    // 取所有应用详细信息
    void getAppsData();

    // 外部调用刷新表格数据
    void refreshData(bool bAdd, QString sID);

private:
    // 初始化界面
    void initUI();

    // 加再数据到Map容器 进行数据处理
    void loadMap(QMap<QString, QStringList>);
    // 将处理后的数据加载到QTableView表格
    void loadAppData(int nRow, QString sPath, QString sName, QString sIcon, QString sId, QString sRealName);

    // 设置统计行信息
    void showTotalInfo();

    // 按钮点击后改变相应行的显示数据和状态
    void changeItemStatus(bool isStartup, QString sFlagData);

private:
    AutoStartModel *m_model;
    QLabel *m_detailInfo;
    DTableView *m_table_view; // 表格
    StartupModel *m_item_model; // QT表格模型
    MyItemDelegate *m_delegate; // 表格代理
    DefenderTable *m_tableWidget;

    int m_rowCount; // 表格加载数据总行数
    QMap<QString, QStringList> m_mapEnable; // 不允许自启动数据容器
    QMap<QString, QStringList> m_mapDisable; // 允许自启动数据容器
    QMap<QString, QString> m_appNamefromId;

    int m_nSaveSortColumn;
    Qt::SortOrder m_nSaveSortOrder = Qt::DescendingOrder;
};

class StartupMainWindow : public DefSecurityToolsMnd
{
    Q_OBJECT
public:
    explicit StartupMainWindow(QWidget *pParent = nullptr);
    ~StartupMainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

Q_SIGNALS:
    void sendWindowClose();

private:
    StartupWidget *m_pStartupWidget;
};
