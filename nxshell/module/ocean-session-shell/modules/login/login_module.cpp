// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "login_module.h"

#include <QBoxLayout>
#include <QWidget>

#include <QtWebEngineWidgets/QWebEngineView>

#include <QtWebChannel/QWebChannel>

namespace dss {
namespace module {

LoginModule::LoginModule(QObject *parent)
    : QObject(parent)
    , m_callbackFun(nullptr)
    , m_callbackData(new AuthCallbackData)
    , m_loginWidget(nullptr)
{
    setObjectName(QStringLiteral("LoginModule"));

    m_callbackData->account = "uos";
    m_callbackData->token = "1";
    m_callbackData->result = 0;
}

LoginModule::~LoginModule()
{
    if (m_callbackData) {
        delete m_callbackData;
    }
}

void LoginModule::init()
{
    initUI();
}

void LoginModule::initUI()
{
    if (m_loginWidget) {
        return;
    }
    m_loginWidget = new QWidget;
    m_loginWidget->setAccessibleName(QStringLiteral("LoginWidget"));
    m_loginWidget->setMinimumSize(280, 280);
    QVBoxLayout *loginLayout = new QVBoxLayout(m_loginWidget);

    QWebEngineView *webView = new QWebEngineView(m_loginWidget);
    QWebEnginePage *webPage = new QWebEnginePage(m_loginWidget);
    QWebChannel *webChannel = new QWebChannel(m_loginWidget);
    webView->setPage(webPage);
    webPage->setWebChannel(webChannel);
    webView->setContextMenuPolicy(Qt::NoContextMenu);

    loginLayout->addWidget(webView);

    webView->load(QUrl("https://login.uniontech.com/"));
}

void LoginModule::setAuthFinishedCallback(AuthCallback *callback)
{
    m_callback = callback;
    m_callbackFun = callback->callbackFun;

    // 当认证完成，调用这个方法
    m_callbackFun(m_callbackData, m_callback->app_data);
}

} // namespace module
} // namespace dss
