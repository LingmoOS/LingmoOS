// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_memorybutton.h"
#include "../../src/control/memorybutton.h"

Ut_MemoryButton::Ut_MemoryButton()
{

}

TEST_F(Ut_MemoryButton, initanimate)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    m_memorybutton->m_widgetbtn = false;
    m_memorybutton->animate(false, 100);
    m_memorybutton->init();
    EXPECT_EQ(m_memorybutton->m_font.pixelSize(), 16);
    EXPECT_EQ(m_memorybutton->m_font.family(), "Noto Sans");
    EXPECT_EQ(m_memorybutton->m_font.styleName(), "Light");
    m_memorybutton->deleteLater();
}

QVariant stub_setting_memoryone(const QString &key)
{
    Q_UNUSED(key);
    return 1;
}

QVariant stub_setting_memoryzero(const QString &key)
{
    Q_UNUSED(key);
    return 0;
}

TEST_F(Ut_MemoryButton, setbtnlight)
{
    MemoryButton *m_memorybutton = new MemoryButton;

    Stub stub;
    stub.set(ADDR(DSettingsAlt, getOption), stub_setting_memoryone);
    m_memorybutton->setbtnlight(true);
    EXPECT_EQ(m_memorybutton->text(), "MH˄");
    m_memorybutton->setbtnlight(false);
    EXPECT_EQ(m_memorybutton->text(), "MH˅");

    Stub stub1;
    stub1.set(ADDR(DSettingsAlt, getOption), stub_setting_memoryzero);
    m_memorybutton->setbtnlight(true);
    EXPECT_EQ(m_memorybutton->text(), "M˄");
    m_memorybutton->setbtnlight(false);
    EXPECT_EQ(m_memorybutton->text(), "M˅");

    DSettingsAlt::deleteInstance();
    delete m_memorybutton;
}

TEST_F(Ut_MemoryButton, showtips)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    m_memorybutton->setText("M+");
    m_memorybutton->m_widgetbtn = false;
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Memory add");
    m_memorybutton->m_widgetbtn = true;
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Add to memory item");


    m_memorybutton->setText("MR");
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Memory recall");

    m_memorybutton->setText("M-");
    m_memorybutton->m_widgetbtn = false;
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Memory subtract");
    m_memorybutton->m_widgetbtn = true;
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Subtract from memory item");

    m_memorybutton->setText("MS");
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Memory store");

    m_memorybutton->setText("MC");
    m_memorybutton->m_widgetbtn = false;
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Clear all memory");
    m_memorybutton->m_widgetbtn = true;
    m_memorybutton->showtips();
    EXPECT_EQ(m_memorybutton->toolTip(), "Clear memory item");
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, setbuttongray)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    m_memorybutton->setbuttongray(true);
    EXPECT_TRUE(m_memorybutton->m_isallgray);
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, mousePressEvent)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_memorybutton->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);

    m_memorybutton->mousePressEvent(m);
    delete m;
    EXPECT_TRUE(m_memorybutton->m_isPress);
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, mouseReleaseEvent)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonRelease,
                                     m_memorybutton->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    m_memorybutton->mouseReleaseEvent(m);
    EXPECT_FALSE(m_memorybutton->m_isPress);
    EXPECT_TRUE(m_memorybutton->m_isacting);
    EXPECT_TRUE(m_memorybutton->m_isHover);
    delete m;
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, enterEvent)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    QEvent *q = new QEvent(QEvent::Type::Enter);
    m_memorybutton->enterEvent(q);
    EXPECT_TRUE(m_memorybutton->m_isHover);
    EXPECT_EQ(m_memorybutton->m_font.pixelSize(), 17);
    delete q;
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, leaveEvent)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    QEvent *q = new QEvent(QEvent::Type::Leave);
    m_memorybutton->leaveEvent(q);
    EXPECT_FALSE(m_memorybutton->m_isHover);
    EXPECT_EQ(m_memorybutton->m_font.pixelSize(), 16);
    delete q;
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, focusOutEvent)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    QFocusEvent *f = new QFocusEvent(QEvent::Type::FocusOut);
    m_memorybutton->focusOutEvent(f);
    delete f;
    m_memorybutton->deleteLater();
    //无ASSERT
}

bool stub_focus_memory()
{
    return true;
}

TEST_F(Ut_MemoryButton, paintEvent)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    QPaintEvent *event = new QPaintEvent(m_memorybutton->rect());
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_memorybutton->setText("MH˅");
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = true;
    m_memorybutton->m_isPress = false;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_memorybutton->setEnabled(false);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = false;
    m_memorybutton->m_isgray = true;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = true;
    m_memorybutton->paintEvent(event);

    //focus状态
    Stub stub;
    stub.set(ADDR(MemoryButton, hasFocus), stub_focus_memory);
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = true;
    m_memorybutton->m_isPress = false;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = true;
    m_memorybutton->paintEvent(event);
    //无ASSERT
    delete event;
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, paintEvent2)
{
    MemoryButton *m_memorybutton = new MemoryButton;
    QPaintEvent *event = new QPaintEvent(m_memorybutton->rect());
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_memorybutton->setText("MH˄");
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = true;
    m_memorybutton->m_isPress = false;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_memorybutton->setEnabled(false);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = false;
    m_memorybutton->m_isgray = true;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = true;
    m_memorybutton->paintEvent(event);

    //focus状态
    Stub stub;
    stub.set(ADDR(MemoryButton, hasFocus), stub_focus_memory);
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = true;
    m_memorybutton->m_isPress = false;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = true;
    m_memorybutton->paintEvent(event);
    //无ASSERT
    delete event;
    m_memorybutton->deleteLater();
}

TEST_F(Ut_MemoryButton, paintEvent3)
{
    MemoryButton *m_memorybutton = new MemoryButton(QString("M+"), true);
    QPaintEvent *event = new QPaintEvent(m_memorybutton->rect());
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = true;
    m_memorybutton->m_isPress = false;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_memorybutton->setEnabled(false);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = false;
    m_memorybutton->m_isgray = true;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = true;
    m_memorybutton->paintEvent(event);

    //focus状态
    Stub stub;
    stub.set(ADDR(MemoryButton, hasFocus), stub_focus_memory);
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = true;
    m_memorybutton->m_isPress = false;
    m_memorybutton->paintEvent(event);
    m_memorybutton->update();
    m_memorybutton->setEnabled(true);
    m_memorybutton->m_isHover = false;
    m_memorybutton->m_isPress = true;
    m_memorybutton->paintEvent(event);
    //无ASSERT
    delete event;
    m_memorybutton->deleteLater();
}
