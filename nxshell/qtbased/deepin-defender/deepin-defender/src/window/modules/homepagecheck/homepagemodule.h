// SPDX-FileCopyrightText: 2010 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "window/modules/common/common.h"
#include "window/interface/moduleinterface.h"
#include "window/interface/frameproxyinterface.h"
#include "window/modules/common/common.h"

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
    inline HomePageModel *getModel() {return m_homePageModel;}

Q_SIGNALS:

public Q_SLOTS:
    // 打开体检首页
    void showHomepage();
    // 打开体检页
    void ShowCheckPage();

private:
    // 首页数据处理对象
    HomePageModel *m_homePageModel;
    // 首页
    HomePage *m_homePage;
    // 体检页
    HomePageCheckWidget  *m_homePagecheckWidget = nullptr;
};
