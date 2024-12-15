// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_textbutton.h"
#include "../../src/control/textbutton.h"

Ut_TextButton::Ut_TextButton()
{

}

TEST_F(Ut_TextButton, init)
{
    TextButton *m_textbutton = new TextButton;
    m_textbutton->setText("+/-");
//    m_textbutton->animate(false, 100);
    m_textbutton->init();
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 20);
    EXPECT_EQ(m_textbutton->m_font.family(), "Noto Sans");
    EXPECT_EQ(m_textbutton->m_font.styleName(), "Light");
    EXPECT_EQ(m_textbutton->m_font.weight(), 2);
    delete m_textbutton;
}

TEST_F(Ut_TextButton, animate)
{
    TextButton *m_textbutton = new TextButton;
    m_textbutton->m_isHover = true;
    m_textbutton->m_isPress = false;
    m_textbutton->animate(false, 100);
    EXPECT_FALSE(m_textbutton->m_isHover);
    EXPECT_TRUE(m_textbutton->m_isPress);
    delete m_textbutton;
}

TEST_F(Ut_TextButton, setButtonDown)
{
    TextButton *m_textbutton = new TextButton;
    m_textbutton->m_Btnisdown = true;
    m_textbutton->setButtonDown(false);
    EXPECT_FALSE(m_textbutton->m_Btnisdown);
    delete m_textbutton;
}

TEST_F(Ut_TextButton, mousePressEvent)
{
    TextButton *m_textbutton = new TextButton;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_textbutton->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_textbutton->mousePressEvent(m);
    EXPECT_TRUE(m_textbutton->m_isPress);
    EXPECT_FALSE(m_textbutton->m_isHover);
    delete m;
    delete m_textbutton;
}

TEST_F(Ut_TextButton, mouseReleaseEvent)
{
    TextButton *m_textbutton = new TextButton;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonRelease,
                                     m_textbutton->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_textbutton->mouseReleaseEvent(m);
    EXPECT_FALSE(m_textbutton->m_isPress);
    EXPECT_TRUE(m_textbutton->m_isHover);
    delete m;
    delete m_textbutton;
}

TEST_F(Ut_TextButton, enterEvent)
{
    TextButton *m_textbutton = new TextButton;
    QEvent *e = new QEvent(QEvent::Type::Enter);
    m_textbutton->setText("Rand");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 17);
    m_textbutton->setText("logᵧx");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("log");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 18);
    m_textbutton->setText("|x|");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("Mod");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("exp");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 18);
    m_textbutton->setText("sin");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 18);
    m_textbutton->setText("cos");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 18);
    m_textbutton->setText("tan");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 18);
    m_textbutton->setText("cot");
    m_textbutton->enterEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 18);
    m_textbutton->setText("+/-");
    m_textbutton->enterEvent(e);
    EXPECT_TRUE(m_textbutton->m_isHover);
    delete e;
    delete m_textbutton;
}

TEST_F(Ut_TextButton, leaveEvent)
{
    TextButton *m_textbutton = new TextButton;
    QEvent *e = new QEvent(QEvent::Type::Leave);
    m_textbutton->setText("Rand");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 15);
    m_textbutton->setText("logᵧx");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 14);
    m_textbutton->setText("log");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("|x|");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 14);
    m_textbutton->setText("Mod");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 14);
    m_textbutton->setText("exp");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("sin");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("cos");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("tan");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("cot");
    m_textbutton->leaveEvent(e);
    EXPECT_EQ(m_textbutton->m_font.pixelSize(), 16);
    m_textbutton->setText("+/-");
    m_textbutton->leaveEvent(e);
    EXPECT_FALSE(m_textbutton->m_isHover);
    delete e;
    delete m_textbutton;
}

bool stub_focus_text()
{
    return true;
}

TEST_F(Ut_TextButton, paintEvent)
{
    TextButton *m_textbutton = new TextButton("1");
    QPaintEvent *event = new QPaintEvent(m_textbutton->rect());
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_textbutton->m_isHover = true;
    m_textbutton->m_isPress = false;
    m_textbutton->paintEvent(event);
    m_textbutton->update();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_textbutton->m_isHover = false;
    m_textbutton->m_isPress = true;
    m_textbutton->paintEvent(event);
    m_textbutton->update();
    m_textbutton->m_isHover = false;
    m_textbutton->m_isPress = false;
    m_textbutton->paintEvent(event);
    m_textbutton->update();

    //focus状态
    Stub stub;
    stub.set(ADDR(TextButton, hasFocus), stub_focus_text);
    m_textbutton->setEnabled(true);
    m_textbutton->m_isHover = true;
    m_textbutton->m_isPress = false;
    m_textbutton->paintEvent(event);
    m_textbutton->update();
    m_textbutton->setEnabled(true);
    m_textbutton->m_isHover = false;
    m_textbutton->m_isPress = true;
    m_textbutton->paintEvent(event);
    m_textbutton->update();
    m_textbutton->m_isHover = false;
    m_textbutton->m_isPress = false;
    m_textbutton->paintEvent(event);
    delete event;
    //无ASSERT
    delete m_textbutton;
}

TEST_F(Ut_TextButton, keyPressEvent)
{
    TextButton *m_textbutton = new TextButton;
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent *k1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent *k2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent *k3 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent *k4 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QKeyEvent *k5 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    m_textbutton->keyPressEvent(k);
    m_textbutton->keyPressEvent(k1);
    m_textbutton->keyPressEvent(k2);
    m_textbutton->keyPressEvent(k3);
    m_textbutton->keyPressEvent(k4);
    m_textbutton->keyPressEvent(k5);
    //无ASSERT
    delete k;
    delete k1;
    delete k2;
    delete k3;
    delete k4;
    delete k5;
    delete m_textbutton;
}

TEST_F(Ut_TextButton, paintspecialbtn)
{
    TextButton *m_textbutton = new TextButton;
    m_textbutton->m_btn = 1;
    m_textbutton->m_isHover = true;
    m_textbutton->m_isPress = true;
    QPainter painter(m_textbutton);
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 2;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 3;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 4;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 5;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 6;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 7;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 8;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    m_textbutton->m_btn = 9;
    m_textbutton->paintspecialbtn(painter, m_textbutton->rect(), painter.fontMetrics().boundingRect(0, 0, int(m_textbutton->rect().width()), int(m_textbutton->rect().height()),
                                                                                                    Qt::AlignCenter, m_textbutton->text()));
    //paint函数，无assert
    delete m_textbutton;
}
