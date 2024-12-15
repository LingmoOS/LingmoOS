// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHANGEPASSWORDWIDGET_H
#define CHANGEPASSWORDWIDGET_H

#include <QWidget>

#include <dtkwidget_global.h>
#include <dwidgetstype.h>

#include <memory>

#include "userinfo.h"

class DLineEditEx;
class QVBoxLayout;
class UserAvatar;
class PasswordLevelWidget;
DWIDGET_BEGIN_NAMESPACE
class DPasswordEdit;
class DLabel;
DWIDGET_END_NAMESPACE
DWIDGET_USE_NAMESPACE

class ChangePasswordWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChangePasswordWidget(std::shared_ptr<User> user, QWidget *parent = nullptr);

private Q_SLOTS:
    void onNewPasswordTextChanged(const QString &text);
    void onRepeatPasswordTextEdited(const QString &text);
    void onRepeatPasswordEditFinished();
    void onPasswordHintsChanged(const QString &text);
    void onOkClicked();

Q_SIGNALS:
    void changePasswordSuccessed();

private:
    void initUI();
    void initConnections();
    bool isInfoValid();
    void parseProcessResult(int exitCode, const QString &output);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    std::shared_ptr<User> m_user;

    QVBoxLayout *m_mainLayout;
    DLabel *m_lockBtn;
    UserAvatar *m_avatar;
    DLabel *m_nameLabel;
    DLabel *m_tipsLabel;
    DPasswordEdit *m_oldPasswdEdit;
    DPasswordEdit *m_newPasswdEdit;
    PasswordLevelWidget *m_levelWidget;
    DPasswordEdit *m_repeatPasswdEdit;
    DLineEditEx *m_passwordHints;
    DPushButton *m_okBtn;
};

#endif // CHANGEPASSWORDWIDGET_H
