// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYTOOLSWIDGET_H
#define SECURITYTOOLSWIDGET_H

#include "window/modules/common/common.h"
#include "datainterface_interface.h"

#include <DWidget>
#include <DIconButton>
#include <DGuiApplicationHelper>

// 病毒查杀类型
enum SecurityClickType {
    DataUsageItem, //  流量详情
    StartupItem, //  自启动项
    NetControlItem, //  联网应用
    UsbControl,
    LoginSafety
};

using DataInterFaceServer = com::deepin::defender::datainterface;
class QStandardItemModel;

DWIDGET_USE_NAMESPACE
class SecurityToolsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SecurityToolsWidget(DWidget *parent = nullptr);
    ~SecurityToolsWidget();

private:
    // 初始化界面
    void initUI();

private Q_SLOTS:
    // 流量/自启动/联网应用点击槽
    void doSecurityItemClicked(QModelIndex nIndex);
    //设置因主题改变图标
    void changeThemeType(ColorType themeType);

Q_SIGNALS:
    void notifyShowStartupWidget();
    void notifyShowNetControlWidget();
    void notifyShowDataUsageWidget();
    void notifyShowUsbControlWidget();
    void notifyShowLoginSafetyWidget();

private:
    QStandardItemModel *m_buttonModel;
    DGuiApplicationHelper *m_guiHelper; // 方便得到系统主题
    QStringList m_iconList;

    DataInterFaceServer *m_dataInterFaceServer; // 安全中心数据服务
};

#endif // SECURITYTOOLSWIDGET_H
