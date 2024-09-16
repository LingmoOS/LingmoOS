// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "auth_custom.h"

#include "modules_loader.h"

#include <QBoxLayout>

using namespace dss::module;

AuthCustom::AuthCustom(QWidget *parent)
    : AuthModule(AuthCommon::AT_Custom, parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_module(nullptr)
{
    setObjectName(QStringLiteral("AuthCutom"));
    setAccessibleName(QStringLiteral("AuthCutom"));

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
}

void AuthCustom::setModule(dss::module::LoginModuleInterface *module)
{
    if (m_module) {
        return;
    }
    m_module = module;

    init();
}

void AuthCustom::init()
{
    m_authCallback.app_data = this;
    m_authCallback.callbackFun = AuthCustom::authCallBack;
    m_module->setAuthFinishedCallback(&m_authCallback);

    if (m_module->content()->parent() == nullptr) {
        m_mainLayout->addWidget(m_module->content());
    }
}

void AuthCustom::setAuthState(const int state, const QString &result)
{
    qDebug() << "AuthCustom::setAuthState:" << state << result;
    m_state = state;
    switch (state) {
    case AuthCommon::AS_Success:
        emit authFinished(state);
        break;
    default:
        break;
    }
}

void AuthCustom::authCallBack(const AuthCallbackData *callbackData, void *app_data)
{
    qDebug() << "AuthCustom::authCallBack";
    if (callbackData && callbackData->result == 1) {
        emit static_cast<AuthCustom *>(app_data)->requestAuthenticate();
    }
}
