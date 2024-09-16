// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_memhiswidget.h"

#include "../../src/widgets/memhiswidget.h"
#include "../../src/memorypublic.h"
#include <DButtonBox>


Ut_MemHisWidget::Ut_MemHisWidget()
{

}

TEST_F(Ut_MemHisWidget, space)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    m_memhiswidget->m_memoryBtn->keyPressEvent(k);
    EXPECT_EQ(m_memhiswidget->m_stackWidget->currentIndex(), 0);
    m_memhiswidget->m_clearButton->keyPressEvent(k);
    EXPECT_TRUE(m_memhiswidget->m_clearButton->isHidden());
    m_memhiswidget->m_stackWidget->setCurrentWidget(m_memhiswidget->m_listView);
    m_memhiswidget->m_clearButton->keyPressEvent(k);
    EXPECT_FALSE(m_memhiswidget->m_isshowH);
    delete k;
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, filledMem)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    MemoryPublic *m_mempublic = MemoryPublic::instance();
//    m_memhiswidget->m_memoryWidget->generateData(Quantity(1),false);
    m_mempublic->generateData(Quantity(1));
    EXPECT_TRUE(m_memhiswidget->m_isshowM);
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, emptyMem)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    MemoryPublic *m_mempublic = MemoryPublic::instance();
    m_mempublic->memoryclean();
    EXPECT_FALSE(m_memhiswidget->m_isshowM);
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, hisbtnhidden)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->m_listModel->hisbtnhidden();
    EXPECT_FALSE(m_memhiswidget->m_isshowH);
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, clicked)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->m_historyBtn->clicked();
    EXPECT_EQ(m_memhiswidget->m_stackWidget->currentIndex(), 1);
    m_memhiswidget->m_memoryBtn->clicked();
    EXPECT_EQ(m_memhiswidget->m_stackWidget->currentIndex(), 0);
    m_memhiswidget->m_clearButton->clicked();
    EXPECT_TRUE(m_memhiswidget->m_clearButton->isHidden());
    m_memhiswidget->m_stackWidget->setCurrentWidget(m_memhiswidget->m_listView);
    m_memhiswidget->m_clearButton->clicked();
    EXPECT_FALSE(m_memhiswidget->m_isshowH);
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, focusOnButtonbox)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->focusOnButtonbox(Qt::TabFocusReason);
    m_memhiswidget->m_stackWidget->setCurrentIndex(0);
    EXPECT_TRUE(m_memhiswidget->m_buttonBox->button(0)->isChecked());
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, resetFocus)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->m_isshowH = false;
    m_memhiswidget->m_isshowM = true;
    m_memhiswidget->resetFocus();
    EXPECT_EQ(m_memhiswidget->m_memoryWidget->focusPolicy(), Qt::TabFocus);
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, keyPressEvent)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->m_historyBtn->setFocus();
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    QKeyEvent *k1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QKeyEvent *k2 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    m_memhiswidget->keyPressEvent(k);
    m_memhiswidget->keyPressEvent(k1);
    m_memhiswidget->keyPressEvent(k2);
    delete k;
    delete k1;
    delete k2;
    //焦点设置函数，无assert
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, eventFilter)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    QFocusEvent *f = new QFocusEvent(QEvent::Type::FocusIn, Qt::TabFocusReason);
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QFocusEvent *f1 = new QFocusEvent(QEvent::Type::FocusOut, Qt::MouseFocusReason);
    m_memhiswidget->eventFilter(m_memhiswidget->m_historyBtn, static_cast <QEvent *>(f));
    m_memhiswidget->eventFilter(m_memhiswidget->m_memoryBtn, static_cast <QEvent *>(k));
    m_memhiswidget->eventFilter(m_memhiswidget->m_clearButton, static_cast <QEvent *>(k));
    m_memhiswidget->eventFilter(m_memhiswidget->m_memoryWidget, static_cast <QEvent *>(k));
    m_memhiswidget->m_clearButton->setHidden(false);
    m_memhiswidget->eventFilter(m_memhiswidget, static_cast <QEvent *>(k));
    m_memhiswidget->eventFilter(m_memhiswidget, static_cast <QEvent *>(f1));
    m_memhiswidget->eventFilter(m_memhiswidget->m_clearButton, static_cast <QEvent *>(f1));
    EXPECT_FALSE(m_memhiswidget->hasFocus());
    delete f;
    delete f1;
    delete k;
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, focusInEvent)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->m_stackWidget->setCurrentIndex(0);
    QFocusEvent *f = new QFocusEvent(QEvent::Type::FocusIn, Qt::TabFocusReason);
    m_memhiswidget->focusInEvent(f);
    m_memhiswidget->m_stackWidget->setCurrentIndex(1);
    m_memhiswidget->focusInEvent(f);
    delete f;
    //焦点设置函数，无assert
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, themeChanged)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->themeChanged(2);
    EXPECT_EQ(m_memhiswidget->m_themeType, 2);
    EXPECT_EQ(m_memhiswidget->m_clearButton->m_currentUrl, ":/assets/images/dark/empty_normal.svg");
    m_memhiswidget->themeChanged(1);
    EXPECT_EQ(m_memhiswidget->m_themeType, 1);
    EXPECT_EQ(m_memhiswidget->m_clearButton->m_currentUrl, ":/assets/images/light/empty_normal.svg");
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, iconChanged)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->iconChanged(1, 0);
    m_memhiswidget->iconChanged(2, 1);
    //设置icon路径，该路径不保存在Qicon中，无assert
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, historyfilled)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    m_memhiswidget->historyfilled();
    EXPECT_TRUE(m_memhiswidget->m_listView->focusPolicy() == Qt::TabFocus);
    EXPECT_TRUE(m_memhiswidget->m_isshowH);
    m_memhiswidget->deleteLater();
}

TEST_F(Ut_MemHisWidget, memoryFunctions)
{
    MemHisWidget *m_memhiswidget = new MemHisWidget;
    MemoryPublic *m_mempub = MemoryPublic::instance();
    m_mempub->generateData(Quantity(1));
    m_memhiswidget->memoryFunctions(MemHisWidget::memoryplus, Quantity(1), 0);
    EXPECT_EQ(m_mempub->m_list.at(0), Quantity(2));
    m_memhiswidget->memoryFunctions(MemHisWidget::memoryminus, Quantity(1), 0);
    EXPECT_EQ(m_mempub->m_list.at(0), Quantity(1));
    m_memhiswidget->memoryFunctions(MemHisWidget::widgetplus, Quantity(1), 0);
    EXPECT_EQ(m_mempub->m_list.at(0), Quantity(2));
    m_memhiswidget->memoryFunctions(MemHisWidget::widgetminus, Quantity(1), 0);
    EXPECT_EQ(m_mempub->m_list.at(0), Quantity(1));
    m_memhiswidget->memoryFunctions(MemHisWidget::memoryclean, Quantity(1), 0);
    EXPECT_TRUE(m_mempub->isEmpty());
    m_memhiswidget->deleteLater();
}
