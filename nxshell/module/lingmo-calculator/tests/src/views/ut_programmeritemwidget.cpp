// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_programmeritemwidget.h"
#include "../../src/views/programmeritemwidget.h"
#include "../../src/dsettings.h"
#include "../../src/memorypublic.h"


Ut_ProgrammerItemWidget::Ut_ProgrammerItemWidget()
{

}

TEST_F(Ut_ProgrammerItemWidget, enterEvent)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    QEvent *e = new QEvent(QEvent::Type::MouseMove);
    m_programmerItemWidget->enterEvent(e);
    EXPECT_TRUE(m_programmerItemWidget->m_ishover);
    delete e;
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, leaveEvent)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    QEvent *e = new QEvent(QEvent::Type::MouseMove);
    m_programmerItemWidget->leaveEvent(e);
    EXPECT_FALSE(m_programmerItemWidget->m_ishover);
    delete e;
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, mousePressEvent)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_programmerItemWidget->pos(), Qt::MouseButton::RightButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_programmerItemWidget->mousePressEvent(m);
    EXPECT_FALSE(m_programmerItemWidget->m_ispress);
    QMouseEvent *m1 = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                      m_programmerItemWidget->pos(), Qt::MouseButton::LeftButton,
                                      Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_programmerItemWidget->mousePressEvent(m1);
    EXPECT_TRUE(m_programmerItemWidget->m_ispress);
    delete m;
    delete m1;
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, mouseReleaseEvent)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    m_programmerItemWidget->m_ispress = true;
    QMouseEvent *e = new QMouseEvent(QMouseEvent::Type::MouseButtonRelease,
                                     m_programmerItemWidget->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_programmerItemWidget->mouseReleaseEvent(e);
    EXPECT_TRUE(m_programmerItemWidget->m_ishover);
    EXPECT_FALSE(m_programmerItemWidget->m_ispress);
    delete e;
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, cleanHoverState)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    m_programmerItemWidget->cleanHoverState();
    EXPECT_FALSE(m_programmerItemWidget->m_ishover);
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, cleanFocusState)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    m_programmerItemWidget->cleanFocusState();
    EXPECT_FALSE(m_programmerItemWidget->m_isfocus);
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, isMarkHide)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    m_programmerItemWidget->isMarkHide(true);
    EXPECT_TRUE(m_programmerItemWidget->m_isMarkHide);
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, initMark)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    m_programmerItemWidget->initMark();
    EXPECT_EQ(m_programmerItemWidget->m_pointVec.at(1), m_programmerItemWidget->m_markmid);
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, themetypechanged)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_programmerItemWidget->themetypechanged(2);
    EXPECT_EQ(m_programmerItemWidget->m_label->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text).name(),
              "#ffffff");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_programmerItemWidget->themetypechanged(1);
    EXPECT_EQ(m_programmerItemWidget->m_label->palette().color(QPalette::ColorGroup::Active, QPalette::ColorRole::Text).name(),
              "#000000");

    ProgrammerItemWidget *m_programmerItemWidget1 = new ProgrammerItemWidget("Circular shift", 50, ":/assets/images/dark/icon_as_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_programmerItemWidget1->themetypechanged(2);
    EXPECT_EQ(m_programmerItemWidget1->m_normalUrl, ":/assets/images/dark/icon_ro_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_programmerItemWidget1->themetypechanged(1);
    EXPECT_EQ(m_programmerItemWidget1->m_normalUrl, ":/assets/images/light/icon_ro_normal.svg");

    ProgrammerItemWidget *m_programmerItemWidget2 = new ProgrammerItemWidget("Arithmetic shift", 50, ":/assets/images/dark/icon_as_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_programmerItemWidget2->themetypechanged(2);
    EXPECT_EQ(m_programmerItemWidget2->m_normalUrl, ":/assets/images/dark/icon_as_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_programmerItemWidget2->themetypechanged(1);
    EXPECT_EQ(m_programmerItemWidget2->m_normalUrl, ":/assets/images/light/icon_as_normal.svg");

    ProgrammerItemWidget *m_programmerItemWidget3 = new ProgrammerItemWidget("Logical shift", 50, ":/assets/images/dark/icon_as_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_programmerItemWidget3->themetypechanged(2);
    EXPECT_EQ(m_programmerItemWidget3->m_normalUrl, ":/assets/images/dark/icon_ls_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_programmerItemWidget3->themetypechanged(1);
    EXPECT_EQ(m_programmerItemWidget3->m_normalUrl, ":/assets/images/light/icon_ls_normal.svg");

    ProgrammerItemWidget *m_programmerItemWidget4 = new ProgrammerItemWidget("Rotate through carry circular shift", 50, ":/assets/images/dark/icon_as_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_programmerItemWidget4->themetypechanged(2);
    EXPECT_EQ(m_programmerItemWidget4->m_normalUrl, ":/assets/images/dark/icon_rc_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_programmerItemWidget4->themetypechanged(1);
    EXPECT_EQ(m_programmerItemWidget4->m_normalUrl, ":/assets/images/light/icon_rc_normal.svg");

    delete m_programmerItemWidget;
    delete m_programmerItemWidget1;
    delete m_programmerItemWidget2;
    delete m_programmerItemWidget3;
    delete m_programmerItemWidget4;
}

TEST_F(Ut_ProgrammerItemWidget, setFocus)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    m_programmerItemWidget->setFocus();
    EXPECT_TRUE(m_programmerItemWidget->m_isfocus);
    delete m_programmerItemWidget;
}

TEST_F(Ut_ProgrammerItemWidget, paintEvent)
{
    ProgrammerItemWidget *m_programmerItemWidget = new ProgrammerItemWidget("WORD");
    QPaintEvent *event = new QPaintEvent(m_programmerItemWidget->rect());
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::UnknownType);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_programmerItemWidget->m_ispress = true;
    m_programmerItemWidget->m_ishover = false;
    m_programmerItemWidget->paintEvent(event);
    m_programmerItemWidget->update();
    ProgrammerItemWidget *m_programmerItemWidget1 = new ProgrammerItemWidget("Circular shift", 50, ":/assets/images/dark/icon_as_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_programmerItemWidget1->m_isMarkHide = false;
    m_programmerItemWidget1->m_isshift = true;
    m_programmerItemWidget1->m_ispress = false;
    m_programmerItemWidget1->m_ishover = true;
    m_programmerItemWidget1->paintEvent(event);
    m_programmerItemWidget1->update();
    delete event;
    //paintevent,无assert
    delete m_programmerItemWidget;
}
