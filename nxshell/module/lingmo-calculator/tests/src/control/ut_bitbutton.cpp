// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_bitbutton.h"
#include "../../src/control/bitbutton.h"

Ut_BitButton::Ut_BitButton()
{

}

TEST_F(Ut_BitButton, init)
{
    BitButton *m_bitbutton = new BitButton;
    m_bitbutton->init();
    EXPECT_EQ(m_bitbutton->m_font.pixelSize(), 18);
    EXPECT_EQ(m_bitbutton->m_font.family(), "Noto Sans");
    EXPECT_EQ(m_bitbutton->m_font.weight(), 2);
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, animate)
{
    BitButton *m_bitbutton = new BitButton;
    m_bitbutton->m_isHover = true;
    m_bitbutton->m_isPress = false;
    m_bitbutton->animate(false, 100);
    EXPECT_FALSE(m_bitbutton->m_isHover);
    EXPECT_TRUE(m_bitbutton->m_isPress);
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, setButtonState)
{
    BitButton *m_bitbutton = new BitButton;
    m_bitbutton->setButtonState(false);
    EXPECT_FALSE(m_bitbutton->m_btnState);
    m_bitbutton->setButtonState(true);
    EXPECT_EQ(m_bitbutton->text(), "1");
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, mousePressEvent)
{
    BitButton *m_bitbutton = new BitButton;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_bitbutton->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_bitbutton->mousePressEvent(m);
    delete m;
    EXPECT_TRUE(m_bitbutton->m_isPress);
    EXPECT_FALSE(m_bitbutton->m_isHover);
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, mouseReleaseEvent)
{
    BitButton *m_bitbutton = new BitButton;
    m_bitbutton->setText("1");
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonRelease,
                                     m_bitbutton->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_bitbutton->mouseReleaseEvent(m);
    EXPECT_FALSE(m_bitbutton->m_isPress);
    EXPECT_FALSE(m_bitbutton->m_btnState);
    delete m;
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, enterEvent)
{
    BitButton *m_bitbutton = new BitButton;
    QEvent *q = new QEvent(QEvent::Type::Enter);

    m_bitbutton->enterEvent(q);
    delete q;
    EXPECT_TRUE(m_bitbutton->m_isHover);
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, leaveEvent)
{
    BitButton *m_bitbutton = new BitButton;
    QEvent *q = new QEvent(QEvent::Type::Leave);

    m_bitbutton->leaveEvent(q);
    delete q;
    EXPECT_FALSE(m_bitbutton->m_isHover);
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, keyPressEvent)
{
    BitButton *m_bitbutton = new BitButton;
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent *k1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent *k2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent *k3 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent *k4 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QKeyEvent *k5 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    m_bitbutton->keyPressEvent(k);
    m_bitbutton->keyPressEvent(k1);
    m_bitbutton->keyPressEvent(k2);
    m_bitbutton->keyPressEvent(k3);
    m_bitbutton->keyPressEvent(k4);
    m_bitbutton->keyPressEvent(k5);
    delete k;
    delete k1;
    delete k2;
    delete k3;
    delete k4;
    delete k5;
    //该函数只修改焦点，无assert
    delete m_bitbutton;
}

TEST_F(Ut_BitButton, focusOutEvent)
{
    BitButton *m_bitbutton = new BitButton;
    QFocusEvent *f = new QFocusEvent(QEvent::Type::FocusOut);
    m_bitbutton->focusOutEvent(f);
    delete f;
    delete m_bitbutton;
    //焦点丢失函数，无assert
}

TEST_F(Ut_BitButton, paintEvent)
{
    BitButton *m_bitbutton = new BitButton;
    QPaintEvent *event = new QPaintEvent(m_bitbutton->rect());
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::UnknownType);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_bitbutton->m_isHover = true;
    m_bitbutton->m_isPress = true;
    m_bitbutton->paintEvent(event);
    m_bitbutton->update();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_bitbutton->paintEvent(event);
    delete event;
    //paintevent,无ASSERT
    delete m_bitbutton;
}
