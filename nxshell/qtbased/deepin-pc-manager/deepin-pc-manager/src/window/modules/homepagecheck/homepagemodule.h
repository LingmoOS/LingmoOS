// Copyright (C) 2010 ~ 2010 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "widgets/checkprocessingwidget.h"
#include "widgets/syscheckresultwidget.h"
#include "window/interface/frameproxyinterface.h"
#include "window/interface/moduleinterface.h"
#include "window/modules/homepagecheck/syscheckadaptormodel.h"

class FrameProxyInterface;
class ModuleInterface;

class QObject;
class HomePageModel;
class HomePage;
class HomePageCheckWidget;
class QObject;

class HomePageModule : public QObject, public ModuleInterface
{
    Q_OBJECT
public:
    explicit HomePageModule(FrameProxyInterface *frame, QObject *parent = nullptr);
    ~HomePageModule() override;

public:
    // 初始化
    void initialize() override;
    // 预初始化
    void preInitialize() override;
    // 模块名称
    const QString name() const override;
    // 激活
    void active(int index) override;
    // 反激活
    void deactive() override;

    // 获得首页数据处理对象
    inline HomePageModel *getModel() { return m_homePageModel; }

Q_SIGNALS:

public Q_SLOTS:
    // 打开体检首页
    void showHomepage();
    // 打开体检页
    void showCheckPage();
    // v23重构，增加进度页
    void showProgressPage();
    // v23重构，打开结果页
    void showResultPage();
    // v23重构，检查中止处理
    void onCheckCanceled();
    // v23重构，重新检查
    void onRequestCheckAgain();
    // back farward
    void requestStopExaming();
    // request quit result widget
    void onRequestQuitResultWidget();
    // sync check score
    void onBackForward();

private:
    // 首页数据处理对象
    HomePageModel *m_homePageModel;
    // model适配器
    SysCheckAdaptorModel *m_sysCheckAdaptorModel;
    // 首页
    HomePage *m_homePage;
    // 体检页
    HomePageCheckWidget *m_homePagecheckWidget = nullptr;
    // 进度页
    CheckProcessingWidget *m_progressWidget;
    // V23结果页
    SysCheckResultWidget *m_resultWidget;
    bool m_isDeactived;
};
