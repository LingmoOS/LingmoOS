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
    , m_loginWidget(nullptr)
{
    setObjectName(QStringLiteral("LoginModule"));
}

LoginModule::~LoginModule()
{
    if (m_loginWidget) {
        delete m_loginWidget;
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
    m_loginWidget->setFixedSize(200, 100);
    QVBoxLayout *loginLayout = new QVBoxLayout(m_loginWidget);

    QWebEngineView *webView = new QWebEngineView(m_loginWidget);
    QWebEnginePage *webPage = new QWebEnginePage(m_loginWidget);
    QWebChannel *webChannel = new QWebChannel(m_loginWidget);
    webView->setPage(webPage);
    webPage->setWebChannel(webChannel);
    webView->setContextMenuPolicy(Qt::NoContextMenu);

    loginLayout->addWidget(webView);

    webView->load(QUrl("https://www.baidu.com"));

    connect(webView, &QWebEngineView::loadFinished, this, [this]{
        QTimer::singleShot(1000, this, [this]{
            AuthCallbackData data;
            data.account = "uos";
            data.token = "1";
            data.result = true;
            m_callbackFun(&data, m_callback->ContainerPtr);
        });

    });
}

void LoginModule::setAuthFinishedCallback(AuthCallback *callback)
{
    m_callback = callback;
    m_callbackFun = callback->callbackFun;
}

} // namespace module
} // namespace dss
