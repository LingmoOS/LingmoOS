// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepagemodule.h"
#include "homepagemodel.h"
#include "homepage.h"
#include "window/modules/homepagecheck/homepagecheckwidget.h"
#include "../src/window/modules/common/invokers/invokerfactory.h"

#include <DGuiApplicationHelper>

#include <QObject>
#include <QTimer>
#include <QModelIndex>
#include <QStandardItemModel>

HomePageModule::HomePageModule(FrameProxyInterface *frame, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frame)
    , m_homePageModel(nullptr)
    , m_homePage(nullptr)
    , m_homePagecheckWidget(nullptr)
{
}

HomePageModule::~HomePageModule()
{
    if (m_homePage) {
        m_homePage->deleteLater();
        m_homePage = nullptr;
    }

    if (m_homePagecheckWidget) {
        m_homePagecheckWidget->deleteLater();
        m_homePagecheckWidget = nullptr;
    }
}

// 初始化
void HomePageModule::initialize()
{
}

// 预初始化
void HomePageModule::preInitialize()
{
    m_homePageModel = new HomePageModel(new InvokerFactory(this), this);
    //
    connect(m_homePageModel, &HomePageModel::notifyShowHomepage, this, &HomePageModule::showHomepage);
}

// 模块名称
const QString HomePageModule::name() const
{
    return MODULE_HOMEPAGE_NAME;
}

// 激活
void HomePageModule::active(int index)
{
    Q_UNUSED(index);
    // 若体检首页没有创建，则新建
    if (nullptr == m_homePage) {
        m_homePage = new HomePage(m_homePageModel);
        connect(m_homePage, &HomePage::notifyShowCheckPage, this, &HomePageModule::ShowCheckPage);
    }
    // 更新体检首页界面数据
    m_homePage->updateUI();
    // 将体检页推到主窗口显示栈
    m_frameProxy->pushWidget(this, m_homePage);

    // 优先显示体检页
    if (m_homePagecheckWidget) {
        ShowCheckPage();
    }
}

// 反激活
void HomePageModule::deactive()
{
}

// 打开体检页
void HomePageModule::ShowCheckPage()
{
    // 若体检页没有创建，则新建
    if (nullptr == m_homePagecheckWidget) {
        m_homePagecheckWidget = new HomePageCheckWidget(m_homePageModel);
    }
    // 隐藏首页
    m_homePage->hide();
    // 将体检页推到主窗口显示栈
    m_homePagecheckWidget->show();
    m_frameProxy->pushWidget(this, m_homePagecheckWidget);
}

// 打开体检首页
void HomePageModule::showHomepage()
{
    // 若体检首页没有创建，则新建
    if (nullptr == m_homePage) {
        m_homePage = new HomePage(m_homePageModel);
        connect(m_homePage, &HomePage::notifyShowCheckPage, this, &HomePageModule::ShowCheckPage);
    }

    // 若体检页已存在，则删除
    if (nullptr != m_homePagecheckWidget) {
        m_frameProxy->popWidget(this);
        m_homePagecheckWidget->deleteLater();
        m_homePagecheckWidget = nullptr;
    }

    m_homePage->updateUI();
    // 将体检页推到主窗口显示栈
    m_frameProxy->pushWidget(this, m_homePage);
}
