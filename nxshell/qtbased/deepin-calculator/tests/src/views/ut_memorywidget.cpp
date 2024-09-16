// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_memorywidget.h"
#include "../../src/views/memorywidget.h"
#include "../../src/memorypublic.h"

#include <QMenu>

Ut_MemoryWidget::Ut_MemoryWidget()
{

}

TEST_F(Ut_MemoryWidget, space)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
//    MemoryWidget *m_memoryWidget = new MemoryWidget;
    QListWidgetItem *item1 = new QListWidgetItem;
    item1->setFlags(Qt::ItemIsEditable);
    MemoryItemWidget *widget = new MemoryItemWidget;
    m_memoryWidget->m_listwidget->insertItem(0, item1);
    m_memoryWidget->m_listwidget->setItemWidget(item1, widget);
    widget->setTextLabel("1");
    m_memoryWidget->m_listwidget->setCurrentRow(0);
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    m_memoryWidget->m_listwidget->keyPressEvent(k);
    m_memoryWidget->m_clearbutton->clicked();
    m_memoryWidget->m_clearbutton->space();
    m_memoryWidget->m_listwidget->itemselected(0, false);
    m_memoryWidget->m_listwidget->focus(0);
    m_memoryWidget->m_listwidget->focus(1);
    EXPECT_TRUE(m_memoryPublic->isEmpty());
    delete k;
}

TEST_F(Ut_MemoryWidget, focus)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryPublic->generateData(Quantity(2));
    m_memoryPublic->generateData(Quantity(3));
    m_memoryWidget->setFocus();
    m_memoryWidget->m_listwidget->setCurrentRow(1);
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QKeyEvent *k1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QKeyEvent *k2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    m_memoryWidget->m_listwidget->keyPressEvent(k);
    m_memoryWidget->m_listwidget->keyPressEvent(k1);
    m_memoryWidget->m_listwidget->keyPressEvent(k2);
    EXPECT_EQ(m_memoryWidget->m_listwidget->currentRow(), 1);
    delete k;
    delete k1;
    delete k2;
}

TEST_F(Ut_MemoryWidget, generateData)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    QTest::mouseClick(static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->m_btnclean, Qt::LeftButton);
    for (int i = 0; i < 500; i++)
        m_memoryPublic->generateData(Quantity(1));
    EXPECT_EQ(m_memoryWidget->m_listwidget->count(), 500);
    m_memoryPublic->generateData(Quantity(2));
    EXPECT_EQ(m_memoryPublic->getList().at(0), Quantity(2));
    m_memoryWidget->m_listwidget->setCurrentRow(1);
    QTest::mouseClick(static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->m_btnplus, Qt::LeftButton);
    QTest::mouseClick(static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->m_btnminus, Qt::LeftButton);

    static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->menucopy();
    static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->menuplus();
    static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->menuclean();
    static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->menuminus();

    EXPECT_EQ(m_memoryWidget->m_listwidget->count(), 499);
}

TEST_F(Ut_MemoryWidget, mousePressEvent)
{
    MemoryWidget *m_memoryWidget = new MemoryWidget;
    QMouseEvent *m = new QMouseEvent(QMouseEvent::Type::MouseButtonPress,
                                     m_memoryWidget->pos(), Qt::MouseButton::LeftButton,
                                     Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier);
    m_memoryWidget->mousePressEvent(m);
    EXPECT_EQ(m_memoryWidget->m_mousepoint, m_memoryWidget->pos());
    delete m;
}

TEST_F(Ut_MemoryWidget, eventFilter)
{
    MemoryWidget *m_memoryWidget = new MemoryWidget;
    m_memoryWidget->m_calculatormode = 0;
    QKeyEvent *k = new QKeyEvent(QKeyEvent::Type::KeyPress, Qt::Key_Tab, Qt::KeyboardModifier::NoModifier);
    m_memoryWidget->eventFilter(m_memoryWidget->m_listwidget, static_cast <QEvent *>(k));
    delete k;
    //焦点设置函数，无assert
}

TEST_F(Ut_MemoryWidget, focusInEvent)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->memoryclean();
    m_memoryPublic->generateData(Quantity(1));
    QFocusEvent *f = new QFocusEvent(QFocusEvent::Type::FocusIn, Qt::TabFocusReason);
    m_memoryWidget->m_currentrow = 0;
    m_memoryWidget->focusInEvent(f);
    delete f;
    EXPECT_EQ(m_memoryWidget->m_listwidget->currentRow(), 0);
}

TEST_F(Ut_MemoryWidget, memoryAnsChanged)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryWidget->memoryAnsChanged(0, Quantity(2));
    EXPECT_EQ(static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->textLabel(), "2");
}

TEST_F(Ut_MemoryWidget, memoryclean)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryWidget->memoryclean();
    EXPECT_FALSE(m_memoryWidget->m_label->isHidden());
    EXPECT_TRUE(m_memoryWidget->m_clearbutton->isHidden());
    EXPECT_EQ(m_memoryWidget->m_listwidget->count(), 1);
}

TEST_F(Ut_MemoryWidget, emptymemoryfontcolor)
{
    MemoryWidget *m_memoryWidget = new MemoryWidget;
    m_memoryWidget->m_themetype = 1;
    m_memoryWidget->emptymemoryfontcolor();
    QColor color(85, 85, 85);
    color.setAlphaF(0.4);
    EXPECT_EQ(m_memoryWidget->m_label->palette().color(DPalette::WindowText), color);

    m_memoryWidget->m_themetype = 2;
    m_memoryWidget->emptymemoryfontcolor();
    QColor color1(192, 198, 212);
    color1.setAlphaF(0.4);
    EXPECT_EQ(m_memoryWidget->m_label->palette().color(DPalette::WindowText), color1);
}

TEST_F(Ut_MemoryWidget, getfirstnumber)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    EXPECT_EQ(m_memoryWidget->getfirstnumber().first, "1");
    EXPECT_EQ(m_memoryWidget->getfirstnumber().second, Quantity(1));
}

TEST_F(Ut_MemoryWidget, getMemoryWidget)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    EXPECT_EQ(static_cast<MemoryItemWidget *>(m_memoryWidget->getMemoryWidget()->itemWidget(m_memoryWidget->m_listwidget->item(0)))->textLabel(), "1");
}

TEST_F(Ut_MemoryWidget, programmerResult)
{
    MemoryWidget *m_memoryWidget = new MemoryWidget;
    HNumber number("1.5");
    EXPECT_EQ(m_memoryWidget->programmerResult(Quantity(number)), "1");
}

TEST_F(Ut_MemoryWidget, resetAllLabelByBase)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    HNumber number("1.7");
    m_memoryPublic->memoryclean();
    m_memoryPublic->generateData(Quantity(number));
    m_memoryWidget->m_calculatormode = 2;
    m_memoryWidget->resetAllLabelByBase();
    EXPECT_EQ(static_cast<MemoryItemWidget *>(m_memoryWidget->getMemoryWidget()->itemWidget(m_memoryWidget->m_listwidget->item(0)))->textLabel(), "1");
}

TEST_F(Ut_MemoryWidget, expressionempty)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryWidget->expressionempty(false);
    EXPECT_FALSE(static_cast<MemoryItemWidget *>(m_memoryWidget->getMemoryWidget()->itemWidget(m_memoryWidget->m_listwidget->item(0)))->m_isExpressionEmpty);
    EXPECT_TRUE(static_cast<MemoryItemWidget *>(m_memoryWidget->getMemoryWidget()->itemWidget(m_memoryWidget->m_listwidget->item(0)))->m_btnplus->isEnabled());
    EXPECT_TRUE(static_cast<MemoryItemWidget *>(m_memoryWidget->getMemoryWidget()->itemWidget(m_memoryWidget->m_listwidget->item(0)))->m_btnminus->isEnabled());
}

TEST_F(Ut_MemoryWidget, widgetcleanslot)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::standardleft);
    m_memoryWidget->memoryclean();
    m_memoryPublic->generateData(Quantity(1));
    m_memoryWidget->widgetcleanslot(0, 1, false);
    EXPECT_EQ(m_memoryWidget->m_listwidget->count(), 1);
    m_memoryWidget->generateData(Quantity(1), false);
    m_memoryWidget->generateData(Quantity(1), false);
    EXPECT_EQ(m_memoryWidget->m_listwidget->count(), 3);
    m_memoryWidget->widgetcleanslot(0, 1, false);
    EXPECT_EQ(m_memoryWidget->m_listwidget->count(), 2);
}

TEST_F(Ut_MemoryWidget, formatExpression)
{
    MemoryWidget *m_memoryWidget = new MemoryWidget;
    EXPECT_EQ(m_memoryWidget->formatExpression("123＋2321"), "123+2321");
}

TEST_F(Ut_MemoryWidget, setitemwordwrap)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryWidget->m_calculatormode = 2;
    m_memoryWidget->setitemwordwrap("12345678901234", 0);
    EXPECT_EQ(m_memoryWidget->m_line, 1);
    m_memoryWidget->setitemwordwrap("1111 1111 1111 1111 1111 1111 1111 1111 1111", 0);
    EXPECT_EQ(m_memoryWidget->m_line, 2);
    m_memoryWidget->m_calculatormode = 0;
    m_memoryWidget->setitemwordwrap("12345678901234E", 0);
    EXPECT_EQ(m_memoryWidget->m_line, 2);
    m_memoryWidget->setitemwordwrap("1234567890123456789012", 0);
    EXPECT_EQ(m_memoryWidget->m_line, 2);
    m_memoryWidget->m_calculatormode = 1;
    m_memoryWidget->setitemwordwrap("123456789012345678901E", 0);
    EXPECT_EQ(m_memoryWidget->m_line, 2);
    m_memoryWidget->setitemwordwrap("1234567890123456789012", 0);
    EXPECT_EQ(m_memoryWidget->m_line, 2);
}

TEST_F(Ut_MemoryWidget, setThemeType)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryWidget->setThemeType(2);
    EXPECT_EQ(m_memoryWidget->m_themetype, 2);
    EXPECT_EQ(m_memoryWidget->m_clearbutton->m_currentUrl, ":/assets/images/dark/empty_normal.svg");
    m_memoryWidget->setThemeType(1);
    EXPECT_EQ(m_memoryWidget->m_themetype, 1);
    EXPECT_EQ(m_memoryWidget->m_clearbutton->m_currentUrl, ":/assets/images/light/empty_normal.svg");
}

TEST_F(Ut_MemoryWidget, keyPressEvent)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_memoryWidget->keyPressEvent(k);
    delete k;
    EXPECT_FALSE(m_memoryWidget->isHidden());
}

TEST_F(Ut_MemoryWidget, resetItemHeight)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *m_memoryWidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryWidget->m_calculatormode = 1;
    m_memoryPublic->generateData(Quantity(1));
    m_memoryWidget->resetItemHeight();
    int line = static_cast<MemoryItemWidget *>(m_memoryWidget->m_listwidget->itemWidget(m_memoryWidget->m_listwidget->item(0)))->getLine();
    EXPECT_EQ(line, 1);
}
