// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "homepagemodule.h"

#include "homepage.h"
#include "homepagemodel.h"

#include <DGuiApplicationHelper>

#include <QModelIndex>
#include <QObject>
#include <QStandardItemModel>
#include <QTimer>

#define AUTO_RESET_WIDGET(className, widget)                  \
    {                                                         \
        connect(widget, &className::destroyed, this, [this] { \
            widget = nullptr;                                 \
        });                                                   \
    }

HomePageModule::HomePageModule(FrameProxyInterface *frame, QObject *parent)
    : QObject(parent)
    , ModuleInterface(frame)
    , m_homePageModel(nullptr)
    , m_sysCheckAdaptorModel(nullptr)
    , m_homePage(nullptr)
    , m_homePagecheckWidget(nullptr)
    , m_progressWidget(nullptr)
    , m_resultWidget(nullptr)
    , m_isDeactived(true)
{
}

HomePageModule::~HomePageModule()
{
    if (m_progressWidget) {
        m_sysCheckAdaptorModel->onCheckCanceled();
        m_progressWidget->deleteLater();
        m_progressWidget = nullptr;
    }

    if (m_resultWidget) {
        m_resultWidget->deleteLater();
        m_resultWidget = nullptr;
    }

    if (m_homePage) {
        m_homePage->deleteLater();
        m_homePage = nullptr;
    }
}

// 初始化
void HomePageModule::initialize() { }

// 预初始化
void HomePageModule::preInitialize()
{
    if (!m_homePageModel) {
        m_homePageModel = new HomePageModel(this);
    }

    if (!m_sysCheckAdaptorModel) {
        m_sysCheckAdaptorModel = new SysCheckAdaptorModel(m_homePageModel, this);
        connect(m_sysCheckAdaptorModel, &SysCheckAdaptorModel::checkFinished, this, [this] {
            // 展示结果页
            showResultPage();
        });

        connect(m_sysCheckAdaptorModel, &SysCheckAdaptorModel::fixItemFinished, this, [this] {
            if (m_resultWidget) {
                m_resultWidget->setFastFixValid(m_sysCheckAdaptorModel->isFastFixValid());
            }
        });

        connect(m_sysCheckAdaptorModel, &SysCheckAdaptorModel::fixItemStarted, this, [this] {
            if (m_frameProxy) {
                m_frameProxy->setBackForwardButtonStatus(false);
            }
        });
        connect(m_sysCheckAdaptorModel, &SysCheckAdaptorModel::fixItemFinished, this, [this] {
            if (m_frameProxy) {
                if (!m_sysCheckAdaptorModel->isProcessingFastFixing()) {
                    m_frameProxy->setBackForwardButtonStatus(true);
                }
            }
        });
    }
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
    m_isDeactived = false;
    // 若体检首页没有创建，则新建
    if (nullptr == m_homePage) {
        m_homePage = new HomePage(m_homePageModel);
        AUTO_RESET_WIDGET(HomePage, m_homePage);

        connect(m_homePage,
                &HomePage::notifyShowCheckPage,
                this,
                &HomePageModule::showProgressPage);
    }

    // 优先显示检查过程（如果处理检查中）
    if (m_progressWidget) {
        m_homePage->hide();
        m_frameProxy->pushWidget(this, m_progressWidget);
        return;
    }

    // 更新体检首页界面数据
    m_homePage->updateUI();
    // 将体检页推到主窗口显示栈
    m_frameProxy->pushWidget(this, m_homePage);

    // 优先显示体检页（如果已产生体检结果）
    if (m_resultWidget) {
        m_homePage->hide();
        m_frameProxy->pushWidget(this, m_resultWidget);
        m_frameProxy->setBackForwardButtonStatus(!m_sysCheckAdaptorModel->isProcessingFastFixing());
        return;
    }
}

// 反激活
void HomePageModule::deactive()
{
    // m_frameProxy->popWidget(this);
    m_isDeactived = true;
}

// 打开体检页
// 因重构弃用
void HomePageModule::showCheckPage() { }

void HomePageModule::showProgressPage()
{
    // 若体检页没有创建，则新建
    if (nullptr == m_progressWidget) {
        m_progressWidget = new CheckProcessingWidget;
        connect(m_progressWidget,
                &CheckProcessingWidget::checkCanceled,
                this,
                &HomePageModule::onCheckCanceled);
        connect(m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::checkStarted,
                m_progressWidget,
                &CheckProcessingWidget::onCheckInitial);
        connect(m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::checkFinished,
                m_progressWidget,
                &CheckProcessingWidget::onCheckDone);
        connect(m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::stageChanged,
                m_progressWidget,
                &CheckProcessingWidget::onCheckMissionStarted);

        AUTO_RESET_WIDGET(CheckProcessingWidget, m_progressWidget);
    }
    // 隐藏首页
    m_frameProxy->popWidget(this);
    m_frameProxy->pushWidget(this, m_progressWidget);
    m_sysCheckAdaptorModel->startExam();
}

void HomePageModule::showResultPage()
{
    if (!m_resultWidget) {
        m_resultWidget = new SysCheckResultWidget;
        m_resultWidget->setResultModel(m_sysCheckAdaptorModel->getResultModel());
        m_resultWidget->setHeaderContent(m_sysCheckAdaptorModel->getIssuePoint(),
                                         m_sysCheckAdaptorModel->getIssueCount());

        connect(m_resultWidget,
                &SysCheckResultWidget::requestSetIgnore,
                m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::onRequestSetIgnore);
        connect(m_resultWidget,
                &SysCheckResultWidget::requestFixAll,
                m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::onRequestFixAll);
        connect(m_resultWidget,
                &SysCheckResultWidget::requestFixItem,
                m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::onRequestFixItem);
        connect(m_resultWidget,
                &SysCheckResultWidget::requestCheckAgain,
                this,
                &HomePageModule::onRequestCheckAgain);
        connect(m_resultWidget,
                &SysCheckResultWidget::requestQuit,
                this,
                &HomePageModule::onRequestQuitResultWidget);

        connect(m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::fixItemStarted,
                m_resultWidget,
                &SysCheckResultWidget::onFixStarted);
        connect(m_sysCheckAdaptorModel,
                &SysCheckAdaptorModel::fixItemFinished,
                m_resultWidget,
                &SysCheckResultWidget::onFixFinished);

        AUTO_RESET_WIDGET(SysCheckResultWidget, m_resultWidget);
    }
    m_resultWidget->setFastFixValid(m_sysCheckAdaptorModel->isFastFixValid());

    if (!m_isDeactived) {
        // 删除过程页
        m_frameProxy->popAndDelWidget(this);
        //  插入主页与结果页
        m_frameProxy->pushWidget(this, m_homePage);
        m_homePage->hide();
        m_frameProxy->pushWidget(this, m_resultWidget);
    } else {
        m_progressWidget->deleteLater();
        m_progressWidget = nullptr;
    }
}

void HomePageModule::onCheckCanceled()
{
    // 通知model取消其它检查
    m_sysCheckAdaptorModel->onCheckCanceled();
    m_homePageModel->notifyStopCheckSysUpdate();
    //  检查中断，回退到前一页面
    m_frameProxy->popAndDelWidget(this);
    m_frameProxy->pushWidget(this, m_homePage);
}

void HomePageModule::onRequestCheckAgain()
{
    m_frameProxy->popAndDelWidget(this);
    showProgressPage();
}

void HomePageModule::requestStopExaming()
{
    m_sysCheckAdaptorModel->onCheckCanceled();
    m_homePageModel->notifyStopCheckSysUpdate();
}

void HomePageModule::onRequestQuitResultWidget()
{
    if (m_resultWidget) {
        if (!m_resultWidget->isHidden()) {
            m_frameProxy->popAndDelWidget(this);
            m_frameProxy->pushWidget(this, m_homePage);
        }
    }
}

void HomePageModule::onBackForward()
{
    if (m_homePage) {
        m_homePage->updateUI();
    }
}

// 打开体检首页
// 因重构弃用
void HomePageModule::showHomepage()
{
    //    // 若体检首页没有创建，则新建
    //    if (nullptr == m_homePage) {
    //        m_homePage = new HomePage(m_homePageModel);
    //        connect(m_homePage, &HomePage::notifyShowCheckPage, this,
    //        &HomePageModule::ShowCheckPage);
    //    }

    //    // 若体检页已存在，则删除
    //    if (nullptr != m_homePagecheckWidget) {
    //        m_frameProxy->popWidget(this);
    //        m_homePagecheckWidget->deleteLater();
    //        m_homePagecheckWidget = nullptr;
    //    }

    //    m_homePage->updateUI();
    //    // 将体检页推到主窗口显示栈
    //    m_frameProxy->pushWidget(this, m_homePage);
}
