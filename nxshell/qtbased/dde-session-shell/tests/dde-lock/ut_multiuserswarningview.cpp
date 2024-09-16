// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "multiuserswarningview.h"

#include <QTest>

#include <gtest/gtest.h>

class UT_MultiUsersWarningView : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    MultiUsersWarningView *m_multiUsersWarningView;
};

class UT_UserListItem : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserListItem *m_userListItem;
};

void UT_MultiUsersWarningView::SetUp()
{
    m_multiUsersWarningView = new MultiUsersWarningView(SessionBaseModel::RequireNormal);
}

void UT_MultiUsersWarningView::TearDown()
{
    delete m_multiUsersWarningView;
}

TEST_F(UT_MultiUsersWarningView, BasicTest)
{
    std::shared_ptr<User> user_ptr(new User());
    m_multiUsersWarningView->action();
    m_multiUsersWarningView->toggleButtonState();
    m_multiUsersWarningView->buttonClickHandle();
    m_multiUsersWarningView->setAcceptReason("test");
    m_multiUsersWarningView->getUserIcon(user_ptr->avatar());
    QTest::keyPress(m_multiUsersWarningView, Qt::Key_Up, Qt::KeyboardModifier::NoModifier);

}

void UT_UserListItem::SetUp()
{
    m_userListItem = new UserListItem("", "");
}

void UT_UserListItem::TearDown()
{
    delete m_userListItem;
}

TEST_F(UT_UserListItem, BasicTest)
{
}
