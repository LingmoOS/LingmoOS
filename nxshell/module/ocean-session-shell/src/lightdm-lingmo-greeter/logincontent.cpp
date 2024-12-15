// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logincontent.h"
#include "logintipswindow.h"
#include "sessionmanager.h"
#include "controlwidget.h"
#include "changepasswordwidget.h"

LoginContent::LoginContent(SessionBaseModel *const model, QWidget *parent)
    : LockContent(model, parent)
{
    setAccessibleName("LoginContent");

    // TODO 确保使用之前先setModel，代码层面有优化空间
    SessionManager::Reference().setModel(model);
    m_controlWidget->setSessionSwitchEnable(SessionManager::Reference().sessionCount() > 1);
    m_controlWidget->chooseToSession(model->sessionKey());
    connect(m_controlWidget, &ControlWidget::requestSwitchSession, &SessionManager::Reference(), &SessionManager::switchSession);
    connect(m_model, &SessionBaseModel::onSessionKeyChanged, m_controlWidget, &ControlWidget::chooseToSession);
    connect(m_model, &SessionBaseModel::requestLoginFrame, this, &LoginContent::pushLoginFrame);
}

void LoginContent::onCurrentUserChanged(std::shared_ptr<User> user)
{
    if (!user.get())
        return;

    LockContent::onCurrentUserChanged(user);
    SessionManager::Reference().updateSession(user->name());
}

void LoginContent::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    // TODO 暂时没有好的处理方式，后期再看这个函数放哪里合适
    if (tryPushTipsFrame())
        return;

    switch (status) {
    case SessionBaseModel::ModeStatus::ResetPasswdMode:
        pushChangePasswordFrame();
        break;
    default:
       break;
    }
}

/**
 * @brief LoginContent::tryPushTipsFrame
 * @note  当配置文件中登陆界面提示信息不为空时，显示此界面
 * @return 是否显示提示信息界面
 */
bool LoginContent::tryPushTipsFrame()
{
    // 如果配置文件存在，并且获取到的内容有效，则显示提示界面，并保证只在greeter启动时显示一次
    static bool showOnce = true;
    m_loginTipsWindow.reset(new LoginTipsWindow);
    if (showOnce && m_loginTipsWindow->isValid()) {
        setTopFrameVisible(false);
        setBottomFrameVisible(false);
        QSize size = getCenterContentSize();
        m_loginTipsWindow->setFixedSize(size);
        setCenterContent(m_loginTipsWindow.get());
        connect(m_loginTipsWindow.get(), &LoginTipsWindow::closed, m_model, &SessionBaseModel::requestLoginFrame);
        showOnce = false;
        return true;
    } else {
        m_loginTipsWindow.reset();
        LockContent::onStatusChanged(m_model->currentModeState());
    }

    return false;
}

void LoginContent::pushLoginFrame()
{
    // 点击确认按钮后显示登录界面
    setTopFrameVisible(true);
    setBottomFrameVisible(true);
    LockContent::onStatusChanged(m_model->currentModeState());
}

/**
 * @brief LoginContent::pushChangePasswordFrame
 * @note  显示重置密码界面，只有当前用户是管理员且密码已过期时会被调用
 */
void LoginContent::pushChangePasswordFrame()
{
    m_resetPasswordWidget.reset(new ChangePasswordWidget(m_model->currentUser(), this));
    connect(m_resetPasswordWidget.get(), &ChangePasswordWidget::changePasswordSuccessed, this, [ = ] {
            // TODO 这里是回到密码输入界面还是某个因子验证的界面
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        });
    setCenterContent(m_resetPasswordWidget.get(), Qt::AlignCenter, 0);

    LockContent::onStatusChanged(m_model->currentModeState());
}
