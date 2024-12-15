// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "daccountsmanager.h"
#include "fakedbus/accountsmanagerservice.h"

DACCOUNTS_USE_NAMESPACE

class TestDAccountsManager : public testing::Test
{
public:
    static void SetUpTestCase()
    {
        m_fakeinter = new FakeAccountsManagerService();
        m_damanager = new DAccountsManager();
    }

    static void TearDownTestCase()
    {
        delete m_fakeinter;
        delete m_damanager;
    }

    void SetUp() override { }

    void TearDown() override { }

    static inline DAccountsManager *m_damanager{ nullptr };
    static inline FakeAccountsManagerService *m_fakeinter{ nullptr };
};

TEST_F(TestDAccountsManager, userList)
{
    EXPECT_EQ(false, m_fakeinter->m_userListTrigger);
    m_damanager->userList();
    EXPECT_EQ(true, m_fakeinter->m_userListTrigger);
}

TEST_F(TestDAccountsManager, createUser)
{
    EXPECT_EQ(false, m_fakeinter->m_createUserTrigger);
    m_damanager->createUser("aaa", "bbb", AccountTypes::Default);
    EXPECT_EQ(true, m_fakeinter->m_createUserTrigger);
}

TEST_F(TestDAccountsManager, deleteUser)
{
    EXPECT_EQ(false, m_fakeinter->m_deleteUserTrigger);
    m_damanager->deleteUser("aaa", true);
    EXPECT_EQ(true, m_fakeinter->m_deleteUserTrigger);
}

TEST_F(TestDAccountsManager, findUserById)
{
    EXPECT_EQ(false, m_fakeinter->m_findUserByIdTrigger);
    m_damanager->findUserById(1000);
    EXPECT_EQ(true, m_fakeinter->m_findUserByIdTrigger);
}

TEST_F(TestDAccountsManager, findUserByName)
{
    EXPECT_EQ(false, m_fakeinter->m_findUserByNameTrigger);
    m_damanager->findUserByName("aaa");
    EXPECT_EQ(true, m_fakeinter->m_findUserByNameTrigger);
}

TEST_F(TestDAccountsManager, presetGroups)
{
    EXPECT_EQ(false, m_fakeinter->m_presetGroupsTrigger);
    m_damanager->presetGroups(AccountTypes::Default);
    EXPECT_EQ(true, m_fakeinter->m_presetGroupsTrigger);
}

TEST_F(TestDAccountsManager, isPasswordValid)
{
    EXPECT_EQ(false, m_fakeinter->m_isPasswordValidTrigger);
    m_damanager->isPasswordValid("12345");
    EXPECT_EQ(true, m_fakeinter->m_isPasswordValidTrigger);
}

TEST_F(TestDAccountsManager, isUsernameValid)
{
    EXPECT_EQ(false, m_fakeinter->m_isUsernameValidTrigger);
    m_damanager->isUsernameValid("aaa");
    EXPECT_EQ(true, m_fakeinter->m_isUsernameValidTrigger);
}
