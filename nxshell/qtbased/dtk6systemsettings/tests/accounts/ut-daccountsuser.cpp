// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "daccountstypes.h"
#include "daccountsuser.h"
#include "fakedbus/accountsuserservice.h"

DACCOUNTS_USE_NAMESPACE

class TestDAccountsUser : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeservice = new FakeAccountsUserService();
        m_dauser = new DAccountsUser(1000);
    }

    static void TearDownTestCase()
    {
        delete m_fakeservice;
        delete m_dauser;
    }

    void SetUp() override { }

    void TearDown() override { }

    static inline DAccountsUser *m_dauser{ nullptr };
    static inline FakeAccountsUserService *m_fakeservice{ nullptr };
};

TEST_F(TestDAccountsUser, accountType)
{
    EXPECT_EQ(AccountTypes::Default, m_dauser->accountType());
    m_fakeservice->m_accountType = 1;
    EXPECT_EQ(1, m_fakeservice->m_accountType);
    EXPECT_EQ(AccountTypes::Admin, m_dauser->accountType());
    m_fakeservice->m_accountType = 4;
    EXPECT_EQ(AccountTypes::Unknown, m_dauser->accountType());
}

TEST_F(TestDAccountsUser, automaticLogin)
{
    EXPECT_FALSE(m_dauser->automaticLogin());
    m_dauser->setAutomaticLogin(true);
    EXPECT_TRUE(m_dauser->automaticLogin());
}

TEST_F(TestDAccountsUser, fullName)
{
    EXPECT_EQ("test1", m_dauser->fullName());
    m_dauser->setFullName("test2");
    EXPECT_EQ("test2", m_dauser->fullName());
}

TEST_F(TestDAccountsUser, layoutList)
{
    QList<QByteArray> tmp{ "cn;" };
    EXPECT_EQ(tmp, m_dauser->layoutList());
    tmp.clear();
    tmp.append({ "en;", "cn;" });
    m_dauser->setLayoutList(tmp);
    EXPECT_EQ(tmp, m_dauser->layoutList());
}

TEST_F(TestDAccountsUser, homeDir)
{
    EXPECT_EQ("/home/test", m_dauser->homeDir());
    m_dauser->setHomeDir("/home/deepin");
    EXPECT_EQ("/home/deepin", m_dauser->homeDir());
}

TEST_F(TestDAccountsUser, iconFile)
{
    EXPECT_EQ("file:///var/lib/AccountsService/icons/11.png", m_dauser->iconFile());
    auto tmp = QUrl::fromLocalFile("/var/lib/AccountsService/icons/12.png");
    m_dauser->setIconFile(tmp);
    EXPECT_EQ("file:///var/lib/AccountsService/icons/12.png", m_dauser->iconFile());
}

TEST_F(TestDAccountsUser, layout)
{
    EXPECT_EQ("cn;", m_dauser->layout());
    m_dauser->setLayout("en;");
    EXPECT_EQ("en;", m_dauser->layout());
}

TEST_F(TestDAccountsUser, locale)
{
    EXPECT_EQ("zh_CN.UTF-8", m_dauser->locale());
    m_dauser->setLocale("en_US.UTF-8");
    EXPECT_EQ("en_US.UTF-8", m_dauser->locale());
}

TEST_F(TestDAccountsUser, locked)
{
    EXPECT_FALSE(m_dauser->locked());
    m_dauser->setLocked(true);
    EXPECT_TRUE(m_dauser->locked());
}

TEST_F(TestDAccountsUser, passwordHint)
{
    EXPECT_EQ("hint", m_dauser->passwordHint());
    m_dauser->setPasswordHint("newhint");
    EXPECT_EQ("newhint", m_dauser->passwordHint());
}

TEST_F(TestDAccountsUser, shell)
{
    EXPECT_EQ("/bin/bash", m_dauser->shell());
    m_dauser->setShell("/bin/zsh");
    EXPECT_EQ("/bin/zsh", m_dauser->shell());
}

TEST_F(TestDAccountsUser, UUID)
{
    EXPECT_EQ("3778df97-91e8-46b2-a136-bfdcdede27fe", m_dauser->UUID());
}

TEST_F(TestDAccountsUser, username)
{
    EXPECT_EQ("test", m_dauser->userName());
}

TEST_F(TestDAccountsUser, noPasswdLogin)
{
    EXPECT_FALSE(m_dauser->noPasswdLogin());
    m_dauser->setNopasswdLogin(true);
    EXPECT_TRUE(m_dauser->noPasswdLogin());
}

TEST_F(TestDAccountsUser, loginTime)
{
    EXPECT_EQ(QDateTime::fromSecsSinceEpoch(1234567), m_dauser->loginTime());
}

TEST_F(TestDAccountsUser, addGroup)
{
    m_dauser->addGroup({});
    EXPECT_TRUE(m_fakeservice->m_addGroupTrigger);
}

TEST_F(TestDAccountsUser, deleteGroup)
{
    m_dauser->deleteGroup({});
    EXPECT_TRUE(m_fakeservice->m_deleteGroupTrigger);
}

TEST_F(TestDAccountsUser, deleteIconFile)
{
    m_dauser->deleteIconFile({});
    EXPECT_TRUE(m_fakeservice->m_deleteIconFileTrigger);
}

TEST_F(TestDAccountsUser, getReminderInfo)
{
    m_dauser->getReminderInfo();
    EXPECT_TRUE(m_fakeservice->m_getReminderInfoTrigger);
}

TEST_F(TestDAccountsUser, setGroups)
{
    m_dauser->setGroups({});
    EXPECT_TRUE(m_fakeservice->m_setGroupsTrigger);
}

TEST_F(TestDAccountsUser, setMaxPasswordAge)
{
    m_dauser->setMaxPasswordAge(10);
    EXPECT_TRUE(m_fakeservice->m_setMaxPasswordAgeTrigger);
}

TEST_F(TestDAccountsUser, setPassword)
{
    m_dauser->setPassword("1234567");
    EXPECT_TRUE(m_fakeservice->m_setPasswordTrigger);
}

TEST_F(TestDAccountsUser, maxPasswordAge)
{
    EXPECT_EQ(99999, m_dauser->maxPasswordAge());
    m_fakeservice->m_maxpasswordage = 0;
    EXPECT_EQ(0, m_dauser->maxPasswordAge());
}

TEST_F(TestDAccountsUser, passwordLastChange)
{
    QDateTime tmp = QDateTime::fromSecsSinceEpoch(0);
    EXPECT_EQ(tmp.addDays(19167), m_dauser->passwordLastChange());
    m_fakeservice->m_passwordlastchange = 0;
    EXPECT_EQ(tmp, m_dauser->passwordLastChange());
}

TEST_F(TestDAccountsUser, passwordStatus)
{
    m_fakeservice->m_locked = true;
    EXPECT_EQ(PasswdStatus::Locked, m_dauser->passwordStatus());
    m_fakeservice->m_locked = false;
    EXPECT_EQ(PasswdStatus::Password, m_dauser->passwordStatus());
    m_fakeservice->m_passwordMode = 2;
    EXPECT_EQ(PasswdStatus::NoPassword, m_dauser->passwordStatus());
    m_fakeservice->m_passwordMode = 5;
    EXPECT_EQ(PasswdStatus::Unknown, m_dauser->passwordStatus());
}

TEST_F(TestDAccountsUser, passwordExpirationInfo)
{
    qint64 tmp;
    m_dauser->passwordExpirationInfo(tmp);
    EXPECT_TRUE(m_fakeservice->m_passwordExpiredInfoTirgger);
}
