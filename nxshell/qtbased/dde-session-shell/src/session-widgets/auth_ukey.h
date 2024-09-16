// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUTHUKEY_H
#define AUTHUKEY_H

#include "auth_module.h"

#include <DPushButton>

#define UKey_Auth QStringLiteral(":/misc/images/auth/ukey.svg")
#define UnionID_Auth QStringLiteral(":/misc/images/auth/UnionID.svg")

class DLineEditEx;
DWIDGET_USE_NAMESPACE

class AuthUKey : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthUKey(QWidget *parent = nullptr);

    void reset();
    QString lineEditText() const;

    void setAnimationState(const bool start) override;
    void setAuthState(const int state, const QString &result) override;
    void setCapsLockVisible(const bool on);
    void setLimitsInfo(const LimitsInfo &info) override;
    void setLineEditEnabled(const bool enable);
    void setLineEditInfo(const QString &text, const TextType type);
    void hide();

signals:
    void focusChanged(const bool);
    void lineEditTextChanged(const QString &); // 数据同步
    void requestChangeFocus();                 // 切换焦点

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initConnections();
    void updateUnlockPrompt() override;

private:
    DLabel *m_capsLock;      // 大小写状态
    DLineEditEx *m_lineEdit; // PIN 输入框
};

#endif // AUTHUKEY_H
