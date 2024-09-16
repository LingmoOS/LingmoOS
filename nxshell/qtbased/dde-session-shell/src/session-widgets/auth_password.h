// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHPASSWORD_H
#define AUTHPASSWORD_H

#include "auth_module.h"

#include <DIconButton>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DFloatingMessage>
#include <DSuggestButton>
#include <DMessageManager>

#define Password_Auth QStringLiteral(":/misc/images/auth/password.svg")
//const QString Password_Auth = ":/misc/images/auth/password.svg";

class DLineEditEx;
DWIDGET_USE_NAMESPACE

class AuthPassword : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthPassword(QWidget *parent = nullptr);

    void reset();
    QString lineEditText() const;

    void setAnimationState(const bool start) override;
    void setAuthState(const int state, const QString &result) override;
    void setCapsLockVisible(const bool on);
    void setLimitsInfo(const LimitsInfo &info) override;
    void setLineEditEnabled(const bool enable);
    void setLineEditInfo(const QString &text, const TextType type);
    void setPasswordHint(const QString &hint);
    void setCurrentUid(uid_t uid);
    void showResetPasswordMessage();
    void closeResetPasswordMessage();

signals:
    void focusChanged(const bool);
    void lineEditTextChanged(const QString &); // 数据同步
    void requestChangeFocus();                 // 切换焦点
    void requestShowKeyboardList();            // 显示键盘布局列表
    void resetPasswordMessageVisibleChanged(const bool isVisible);

public slots:
    void setResetPasswordMessageVisible(const bool isVisible);
    void updateResetPasswordUI();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void initUI();
    void initConnections();
    void updateUnlockPrompt() override;
    void togglePassword();
    void setTogglePasswordBtnIcon();
    void showPasswordHint();
    void setPasswordHintBtnVisible(const bool isVisible);
    bool isUserAccountBinded();

private:
    DLabel *m_capsLock;                 // 大小写状态
    DLineEditEx *m_passwordEdit;        // 密码输入框
    DIconButton *m_passwordHintBtn;     // 密码提示按钮
    QString m_passwordHint;             // 密码提示
    DIconButton *m_togglePasswordBtn;   // 显示/隐藏密码按钮
    bool m_resetPasswordMessageVisible;
    DFloatingMessage *m_resetPasswordFloatingMessage;
    uid_t m_currentUid; // 当前用户uid
    QTimer *m_bindCheckTimer;
};

#endif // AUTHPASSWORD_H
