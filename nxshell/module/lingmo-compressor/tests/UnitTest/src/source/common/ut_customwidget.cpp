// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customwidget.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QTest>


/*******************************函数打桩************************************/

/*******************************单元测试************************************/
// 测试CustomSuggestButton
class UT_CustomSuggestButton : public ::testing::Test
{
public:
    UT_CustomSuggestButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomSuggestButton();
        m_tester1 = new CustomSuggestButton("1");
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_tester1;
    }

protected:
    CustomSuggestButton *m_tester;
    CustomSuggestButton *m_tester1;
};

TEST_F(UT_CustomSuggestButton, initTest)
{

}

TEST_F(UT_CustomSuggestButton, test_keyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(UT_CustomSuggestButton, test_keyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomCombobox
class UT_CustomCombobox : public ::testing::Test
{
public:
    UT_CustomCombobox(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCombobox();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomCombobox *m_tester;
};

TEST_F(UT_CustomCombobox, initTest)
{

}

TEST_F(UT_CustomCombobox, test_keyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(UT_CustomCombobox, test_keyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

TEST_F(UT_CustomCombobox, test_focusInEvent)
{
    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn);
    m_tester->focusInEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_reson, Qt::OtherFocusReason);
}

// 测试CustomPushButton
class UT_CustomPushButton : public ::testing::Test
{
public:
    UT_CustomPushButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomPushButton();
        m_tester1 = new CustomPushButton("1");
        m_tester2 = new CustomPushButton(QIcon(), "1");
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_tester1;
        delete m_tester2;
    }

protected:
    CustomPushButton *m_tester;
    CustomPushButton *m_tester1;
    CustomPushButton *m_tester2;
};

TEST_F(UT_CustomPushButton, initTest)
{

}

TEST_F(UT_CustomPushButton, test_keyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(UT_CustomPushButton, test_keyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomPushButton
class UT_CustomSwitchButton : public ::testing::Test
{
public:
    UT_CustomSwitchButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomSwitchButton();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomSwitchButton *m_tester;
};

TEST_F(UT_CustomSwitchButton, initTest)
{

}

TEST_F(UT_CustomSwitchButton, test_keyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(UT_CustomSwitchButton, test_focusInEvent)
{
    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn);
    m_tester->focusInEvent(event);
    delete event;
    EXPECT_EQ(m_tester->m_reson, Qt::NoFocusReason);
}

// 测试CustomCheckBox
class UT_CustomCheckBox : public ::testing::Test
{
public:
    UT_CustomCheckBox(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCheckBox();
        m_tester1 = new CustomCheckBox("1");
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_tester1;
    }

protected:
    CustomCheckBox *m_tester;
    CustomCheckBox *m_tester1;
};

TEST_F(UT_CustomCheckBox, initTest)
{

}

TEST_F(UT_CustomCheckBox, test_keyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(UT_CustomCheckBox, test_keyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomCommandLinkButton
class UT_CustomCommandLinkButton : public ::testing::Test
{
public:
    UT_CustomCommandLinkButton(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomCommandLinkButton("1");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomCommandLinkButton *m_tester;
};

TEST_F(UT_CustomCommandLinkButton, initTest)
{

}

TEST_F(UT_CustomCommandLinkButton, test_keyPressEvent)
{
    QTest::keyPress(m_tester, Qt::Key_Enter);
    QTest::keyPress(m_tester, Qt::Key_Space);
}

TEST_F(UT_CustomCommandLinkButton, test_keyReleaseEvent)
{
    QTest::keyRelease(m_tester, Qt::Key_Enter);
    QTest::keyRelease(m_tester, Qt::Key_Space);
}

// 测试CustomFloatingMessage
class UT_CustomFloatingMessage : public ::testing::Test
{
public:
    UT_CustomFloatingMessage(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CustomFloatingMessage(QIcon(), "1");
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CustomFloatingMessage *m_tester;
};

TEST_F(UT_CustomFloatingMessage, initTest)
{

}

