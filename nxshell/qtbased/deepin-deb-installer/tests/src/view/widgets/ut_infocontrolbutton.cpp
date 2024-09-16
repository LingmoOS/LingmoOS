// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/widgets/infocontrolbutton.h"
#include "../deb-installer/view/widgets/InfoCommandLinkButton.h"
#include <ut_Head.h>

#include <QMouseEvent>
#include <QKeyEvent>
#include <QCoreApplication>

#include <gtest/gtest.h>

class InfoControlButton_Test : public UT_HEAD
{
public:
    virtual void SetUp() { m_infoControlBtn = new InfoControlButton("", ""); }
    void TearDown() { delete m_infoControlBtn; }
    InfoControlButton *m_infoControlBtn;
};

TEST_F(InfoControlButton_Test, InfoControlButton_UT_setExpandTips)
{
    m_infoControlBtn->setExpandTips("/");
    ASSERT_EQ("/", m_infoControlBtn->m_expandTips);
    ASSERT_EQ("/", m_infoControlBtn->m_tipsText->text());
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_setShrinkTips)
{
    m_infoControlBtn->setShrinkTips("/");
    ASSERT_EQ("/", m_infoControlBtn->m_shrinkTips);
    ASSERT_EQ("/", m_infoControlBtn->m_tipsText->text());
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_onMouseRelease)
{
    m_infoControlBtn->m_expand = false;
    m_infoControlBtn->m_shrinkTips = "shrink";
    m_infoControlBtn->onMouseRelease();
    EXPECT_TRUE(m_infoControlBtn->m_expand);
    ASSERT_EQ("shrink", m_infoControlBtn->m_tipsText->text());
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_onMouseRelease_01)
{
    m_infoControlBtn->m_expand = true;
    m_infoControlBtn->m_expandTips = "expand";
    m_infoControlBtn->onMouseRelease();
    EXPECT_FALSE(m_infoControlBtn->m_expand);
    ASSERT_EQ("expand", m_infoControlBtn->m_tipsText->text());
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_themeChanged)
{
    m_infoControlBtn->themeChanged();
    EXPECT_FALSE(m_infoControlBtn->m_expand) << "false";
    m_infoControlBtn->m_expand = true;
    m_infoControlBtn->themeChanged();
    EXPECT_TRUE(m_infoControlBtn->m_expand) << "true";
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_mouseReleaseEvent)
{
    m_infoControlBtn->m_expand = true;
    m_infoControlBtn->m_expandTips = "expand";
    QMouseEvent mouseReleaseEvent(QEvent::MouseButtonRelease, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_infoControlBtn->mouseReleaseEvent(&mouseReleaseEvent);
    EXPECT_FALSE(m_infoControlBtn->m_expand);
    ASSERT_EQ("expand", m_infoControlBtn->m_tipsText->text());
}

TEST_F(InfoControlButton_Test, InfoControlButton_UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_infoControlBtn, &keyPressEvent);
    EXPECT_FALSE(m_infoControlBtn->hasFocus());
}
