// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PasswordWidget_H
#define PasswordWidget_H

#include <QLabel>

#include <DPasswordEdit>
#include <DGuiApplicationHelper>

#include "deepin_pw_check.h"
#include "utils.h"

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class PasswordWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordWidget(int margin, const QString &userName, QWidget *parent = nullptr);
    ~PasswordWidget() {}

    bool checkPassword();
    inline const QString getPassword() { return m_newPasswordEdit->text(); }
    bool isPasswordEmpty();

private:
    void initWidget(int margin);
    void initData();
    void setPasswdLevelIconModePath(DGuiApplicationHelper::ColorType themeType);
    void hideAlert(DLineEdit *edit);

Q_SIGNALS:
    void requestSetPasswordHint(const QString &);

private Q_SLOTS:
    void onNewPasswordLineEditChanged(const QString&);
    void updatePasswordStrengthLevelWidget(PASSWORD_LEVEL_TYPE level);

private:
    QString m_userName;
    DPasswordEdit *m_newPasswordEdit;
    DPasswordEdit *m_repeatPasswordEdit;
    DLineEdit *m_passwordTipsEdit;
    QLabel *m_newPasswdLevelText;
    QLabel *m_newPasswdLevelIcons[PASSWORD_LEVEL_ICON_NUM];
    PASSWORD_LEVEL_TYPE m_level;
    QString m_newPasswdLevelIconModePath;
};

#endif // REMINDERDDIALOG_H
