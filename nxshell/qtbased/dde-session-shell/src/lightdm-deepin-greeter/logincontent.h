// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGINCONTENT_H
#define LOGINCONTENT_H

#include "lockcontent.h"

class LoginTipsWindow;
class ChangePasswordWidget;
class LoginContent : public LockContent
{
    Q_OBJECT
public:
    explicit LoginContent(SessionBaseModel *const model, QWidget *parent = nullptr);

    void onCurrentUserChanged(std::shared_ptr<User> user) override;
    void onStatusChanged(SessionBaseModel::ModeStatus status) override;

    bool tryPushTipsFrame();
    void pushLoginFrame();
    void pushChangePasswordFrame();

private:
    QSharedPointer<LoginTipsWindow> m_loginTipsWindow;
    QSharedPointer<ChangePasswordWidget> m_resetPasswordWidget;
};

#endif // LOGINCONTENT_H
