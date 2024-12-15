// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#define protected public
#include "networksecretdialog.h"
#undef private
#undef protected

#include <QJsonDocument>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Ut_NetworkSecretDialog : public testing::Test
{
public:
    void SetUp() override;

    void TearDown() override;

public:
    NetworkSecretDialog *dialog = nullptr;
};

void Ut_NetworkSecretDialog::SetUp()
{
   QString json = "{\"connName\":\"UtNetworkSecretDialog\", \"connType\":\"Wiredless\", \"secrets\":[\"wep\"]}";
   dialog = new NetworkSecretDialog(QJsonDocument::fromJson(json.toUtf8()));
}

void Ut_NetworkSecretDialog::TearDown()
{
    if (dialog) {
        delete dialog;
        dialog = nullptr;
    }

}

TEST_F(Ut_NetworkSecretDialog, wepIsValidTest)
{
    EXPECT_FALSE(dialog->wepKeyIsValid("12345678", NetworkSecretDialog::NotSpecified));
    EXPECT_TRUE(dialog->wepKeyIsValid("12345678", NetworkSecretDialog::Passphrase));
}


TEST_F(Ut_NetworkSecretDialog, wpaPskIsValidTest)
{
    EXPECT_TRUE(dialog->wpaPskIsValid("12345678"));
    EXPECT_TRUE(dialog->wpaPskIsValid("0123456789012345678901234567890123456789012345678901234567890123"));
}

TEST_F(Ut_NetworkSecretDialog, submitTest)
{
    //输入无效
    dialog->submit();
    //修改密码
    if (!dialog->m_lineEditList.isEmpty()) {
        auto lineEdit = dialog->m_lineEditList.first();
        lineEdit->setText("12345678");
        Q_EMIT lineEdit->textChanged("12345678");
    }
    //点击确定按钮
    dialog->onButtonsClicked(1, "");
    //点击取消按钮
    dialog->onButtonsClicked(0, "");
}

TEST_F(Ut_NetworkSecretDialog, coverageTest)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_0, Qt::NoModifier);
    dialog->keyPressEvent(&event);
}

