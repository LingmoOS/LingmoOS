// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loginframe.h"
#include "logincontent.h"
#include "userinfo.h"

#include <QWindow>

LoginFrame::LoginFrame(SessionBaseModel *const model, QWidget *parent)
    : FullscreenBackground(model, parent)
    , m_loginContent(new LoginContent(model, this))
    , m_model(model)
{
    setAccessibleName("LoginWindow");
    updateBackground(m_model->currentUser()->greeterBackground());
    setContent(m_loginContent);
    m_loginContent->setVisible(false);

    connect(m_loginContent, &LockContent::requestBackground, this, [ = ](const QString & wallpaper) {
        updateBackground(wallpaper);
        // 在认证成功以后会通过更改背景来实现登录动画，但是禁用登录动画的情况下，会立即调用startSession，
        // 导致当前进程被lightdm退掉，X上会残留上一帧的画面，可以看到输入框等画面。使用repaint()强制刷新背景来避免这个问题。
        repaint();
    });

    connect(model, &SessionBaseModel::authFinished, this, [ = ](bool successful) {
        setEnterEnable(!successful);
        if (successful)
            m_loginContent->setVisible(false);

        // 在认证成功以后会通过更改背景来实现登录动画，但是禁用登录动画的情况下，会立即调用startSession，
        // 导致当前进程被lightdm退掉，X上会残留上一帧的画面，可以看到输入框等画面。使用repaint()强制刷新背景来避免这个问题。
        repaint();
    });

    connect(m_loginContent, &LockContent::requestSwitchToUser, this, &LoginFrame::requestSwitchToUser);
    connect(m_loginContent, &LockContent::requestSetKeyboardLayout, this, &LoginFrame::requestSetKeyboardLayout);

    connect(m_loginContent, &LockContent::requestCheckAccount, this, &LoginFrame::requestCheckAccount);
    connect(m_loginContent, &LockContent::requestStartAuthentication, this, &LoginFrame::requestStartAuthentication);
    connect(m_loginContent, &LockContent::sendTokenToAuth, this, &LoginFrame::sendTokenToAuth);
    connect(m_loginContent, &LockContent::requestEndAuthentication, this, &LoginFrame::requestEndAuthentication);
    connect(m_loginContent, &LockContent::authFinished, this, &LoginFrame::authFinished);
}

void LoginFrame::showEvent(QShowEvent *event)
{
    FullscreenBackground::showEvent(event);

    //greeter界面显示时，需要调用虚拟键盘
    m_model->setHasVirtualKB(true);
    m_model->setVisible(true);
}

void LoginFrame::hideEvent(QHideEvent *event)
{
    FullscreenBackground::hideEvent(event);

    //greeter界面隐藏时，需要结束虚拟键盘
    m_model->setHasVirtualKB(false);
    m_model->setVisible(false);
}
