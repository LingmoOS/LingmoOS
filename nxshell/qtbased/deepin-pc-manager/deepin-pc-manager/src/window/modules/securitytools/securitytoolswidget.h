// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SECURITYTOOLSWIDGET_H
#define SECURITYTOOLSWIDGET_H

#include "securitytoolsmodel.h"
#include "widgets/ddropdown.h"
#include "window/modules/common/common.h"

#include <DButtonBox>
#include <DFrame>
#include <DGuiApplicationHelper>
#include <DIconButton>
#include <DLabel>
#include <DListView>
#include <DStackedWidget>
#include <DWidget>

// 病毒查杀类型
enum SecurityClickType {
    DataUsageItem,      //  流量详情
    StartupItem,        //  自启动项
    NetControlItem,     //  联网应用
    UsbControl,         // USB管控
    LoginSafety,        // 登录安全
    SystemSafeLevel,    // 系统安全等级
    FileTrustProtection // 可信保护
};

class QStandardItemModel;
class SecurityToolsModel;

DWIDGET_USE_NAMESPACE

class SecurityToolsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SecurityToolsWidget(SecurityToolsModel *model, DWidget *parent = nullptr);
    ~SecurityToolsWidget();

private:
    // 初始化界面
    void initUI();
    void initConnection();
    void clearToolInfoList();
    // 更新工具信息
    void updateToolsMsgByStatus(TOOLSSTATUS status);
    void updateToolsMsgByClassify(TOOLCLASSIFY iClassify);
    DButtonBoxButton *createBoxButton(const QString &strText, QWidget *pParent = nullptr);
    DEFSECURITYTOOLINFOLIST getToolInfoByClassify(const DEFSECURITYTOOLINFOLIST &infolist) const;
    DWidget *createEmptyDataPane();

private Q_SLOTS:
    // 设置因主题改变图标
    void changeThemeType(ColorType themeType);

private:
    SecurityToolsModel *m_model;
    DFrame *m_scrollAreaWidgetContents;
    DButtonBox *m_pSecurityButtonBox;
    DLabel *m_pMsgLabel;
    TOOLSSTATUS m_toolStatus;
    TOOLCLASSIFY m_iClassify;
    DDropdown *m_pDropDown;
    DStackedWidget *m_pStackedWidget;
};

#endif // SECURITYTOOLSWIDGET_H
