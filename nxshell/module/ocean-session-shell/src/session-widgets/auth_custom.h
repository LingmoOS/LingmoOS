// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHCUTOM_H
#define AUTHCUTOM_H

#include "auth_module.h"
#include "login_module_interface.h"

#include <QVBoxLayout>

class AuthCustom : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthCustom(QWidget *parent = nullptr);

    void setModule(dss::module::LoginModuleInterface *module);

    void setAuthState(const int state, const QString &result) override;

private:
    void init();
    static void authCallBack(const dss::module::AuthCallbackData *callbackData, void *app_data);

private:
    QVBoxLayout *m_mainLayout;
    dss::module::AuthCallback m_authCallback;
    dss::module::LoginModuleInterface *m_module;
};

#endif // AUTHCUTOM_H
