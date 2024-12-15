// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETCONTROLWIDGET_H
#define NETCONTROLWIDGET_H

#include "dtkwidget_global.h"
#include "window/namespace.h"
#include "defenderprocinfolist.h"
#include "../src/widgets/defendertable.h"

#include <DTipLabel>

#include <QItemDelegate>

using namespace def::widgets;

DEF_NAMESPACE_BEGIN
DEF_NETPROTECTION_NAMESPACE_BEGIN
DWIDGET_USE_NAMESPACE

class DBusInvokerInterface;
class InvokerFactoryInterface;
class SettingsInvokerInterface;
class TipWidget;
class NetControlWidget;

class NetTableViewItemDelegate : public QItemDelegate
{
public:
    NetTableViewItemDelegate(NetControlWidget *widget, QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

private:
    NetControlWidget *m_widget;
};

class NetControlWidget : public DFrame
{
    Q_OBJECT
public:
    explicit NetControlWidget(const QString &sPkgName, InvokerFactoryInterface *invokerFactory, QWidget *parent = nullptr);
    ~NetControlWidget();

    // 提供接口获取图片
    QPixmap getPixmap(const QString &appIcon, const QSize &size);

    // 设置表格选中行
    void setNetControlIndex(const QString &sPkgName);

private:
    // 初始化界面
    void initUi();

    // 初始化数据
    void initData();

    // 初始化判断是否显示联网管控开关未开启提示
    void intiSwitchStatus();

private Q_SLOTS:
    // 接收所有APPS的详细信息
    void doNetAppsInfo(DefenderProcInfoList);

    // 表格内下拉框选项改变
    void onAppOptChanged(int index);

private:
    DTableView *m_tableView; // 表格
    QStandardItemModel *m_tableModel; // 模型
    bool m_bIniFlag; // 排序初始化标志
    NetTableViewItemDelegate *m_tableDelegate; // 表格代理
    QMap<QComboBox *, QString> m_comboToAppName; // 联网管控表格下拉框容器
    QMap<QString, QString> m_appNamefromPkgName;
    QString m_sDefaultStatus; // 联网管控默认状态
    QString m_sLocatPkgName; // 用来定位的包名

    DBusInvokerInterface *m_DataInterFaceServer; // 流量监控dbus类
    DBusInvokerInterface *m_monitorInterFaceServer; // 流量监控dbus类
    SettingsInvokerInterface *m_gsetting; // 安全中心gsetting配置

    DefenderTable *m_netTableWidget;
    DTipLabel *m_lbTotal;
    bool m_isTipDialogFlag;
};

DEF_NAMESPACE_END
DEF_NETPROTECTION_NAMESPACE_END

#endif // NETCONTROLWIDGET_H
