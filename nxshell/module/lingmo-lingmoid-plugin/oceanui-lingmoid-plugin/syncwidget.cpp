// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncwidget.h"
#include "pages/login.h"

#include <QVBoxLayout>
#include <QStackedLayout>
#include <QLabel>
#include <DFloatingButton>
#include <DBackgroundGroup>

DWIDGET_USE_NAMESPACE

/**
 * @brief SyncWidget::SyncWidget  UnionID 二级页面
 */
SyncWidget::SyncWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QStackedLayout)
    , m_model(nullptr)
    , m_loginPage(new LoginPage)
    , m_loginInfoPage(new LoginInfoPage)
    , m_loginInfoDetailPage(new LoginInfoDetailPage)
    , m_groundGroup(new DBackgroundGroup)
    , m_isLogind(false)
{
    initUI();
    initConnect();
}

SyncWidget::~SyncWidget()
{

}

void SyncWidget::setModel(SyncModel *model)
{
    m_model = model;
    m_loginInfoPage->setModel(model);
    m_loginInfoDetailPage->setModel(model);

    connect(model, &SyncModel::userInfoChanged, this, &SyncWidget::onUserInfoChanged);

    onUserInfoChanged(model->userinfo());
}

void SyncWidget::setWorker(SyncWorker *worker)
{
    m_loginInfoDetailPage->setWorker(worker);
}

void SyncWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    //m_loginInfoPage->setMinimumWidth(this->width() / 3);
}

void SyncWidget::initUI()
{
    m_loginInfoPage->setFixedWidth(198);
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    QHBoxLayout *contentLay = new QHBoxLayout;
    contentLay->setContentsMargins(0, 0, 0, 0);
    contentLay->setSpacing(0);
    m_groundGroup->setLayout(contentLay);
    m_groundGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_loginInfoDetailPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_groundGroup->setContentsMargins(0, 0, 0, 0);
    m_groundGroup->setItemSpacing(2);
    m_groundGroup->setBackgroundRole(QPalette::Base);
    m_groundGroup->setUseWidgetBackground(true);

    contentLay->addWidget(m_loginInfoPage);
    contentLay->addWidget(m_loginInfoDetailPage);

    m_mainLayout->addWidget(m_loginPage);
    m_mainLayout->addWidget(m_groundGroup);
    setLayout(m_mainLayout);
}

void SyncWidget::initConnect()
{
    connect(m_loginPage, &LoginPage::requestLoginUser, this, &SyncWidget::requestLoginUser);
    connect(m_loginInfoPage, &LoginInfoPage::requestLogoutUser, this, &SyncWidget::requestAsyncLogoutUser);
    connect(m_loginInfoPage, &LoginInfoPage::requestSetFullname, this, &SyncWidget::requestSetFullname);
    connect(m_loginInfoPage, &LoginInfoPage::requestPullMessage, this, &SyncWidget::requestPullMessage);

    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::requestBindAccount, this, &SyncWidget::requestBindAccount);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::requestUnBindAccount, this, &SyncWidget::requestUnBindAccount);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::requestSetAutoSync, this, &SyncWidget::requestSetAutoSync);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::requestSetModuleState, this, &SyncWidget::requestSetModuleState);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::requestLocalBindCheck, this, &SyncWidget::requestLocalBindCheck);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::requestSetUtcloudModuleState, this, &SyncWidget::requestSetUtcloudModuleState);
    connect(m_loginInfoPage, &LoginInfoPage::selectIndex, m_loginInfoDetailPage, &LoginInfoDetailPage::selectIndex);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::removeDevice, this, &SyncWidget::removeDevice);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::clearCloudData, this, &SyncWidget::clearCloudData);
    connect(m_loginInfoDetailPage, &LoginInfoDetailPage::requestOpenWeb, m_loginInfoPage, &LoginInfoPage::openWeb);
}

void SyncWidget::onUserInfoChanged(const QVariantMap &userInfo)
{
    bool isLogind = !userInfo["Username"].toString().isEmpty();
    if (isLogind) {
        Q_EMIT refreshSyncList();
        Q_EMIT refreshDeviceList();
        m_loginInfoPage->onLogin();
        m_loginInfoDetailPage->onLogin();
        m_mainLayout->setCurrentWidget(m_groundGroup);
        setAutoFillBackground(true);
    } else {
        m_mainLayout->setCurrentWidget(m_loginPage);
        //setBackgroundRole(QPalette::Base);
        setAutoFillBackground(true);
        Q_EMIT m_loginInfoDetailPage->onUserLogout();
    }
}

