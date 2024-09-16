// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_graphicsscene.h"

#include <QDate>
#include <QFocusEvent>
#include <QApplication>
#include <QGraphicsRectItem>
#include "view/graphicsItem/cfocusitem.h"

//test_graphicsscene::test_graphicsscene()
//{
//}

//void test_graphicsscene::SetUp()
//{
//    m_Scene = new CGraphicsScene();
//}

//void test_graphicsscene::TearDown()
//{
//    delete m_Scene;
//    m_Scene = nullptr;
//}

//TEST_F(test_graphicsscene, getCurrentFocusItem)
//{
//    QGraphicsItem *item = m_Scene->getCurrentFocusItem();
//    Q_UNUSED(item)
//}

////setPrePage
//TEST_F(test_graphicsscene, setPrePage)
//{
//    m_Scene->setPrePage(QDate::currentDate(), false);
//}

////focusInDeal
//TEST_F(test_graphicsscene, focusInDeal)
//{
//    m_Scene->setIsShowCurrentItem(true);
//    QFocusEvent event(QEvent::FocusIn, Qt::TabFocusReason);
//    QApplication::sendEvent(m_Scene, &event);
//}

//TEST_F(test_graphicsscene, focusInDeal1)
//{
//    CFocusItem *item = new CFocusItem();
//    m_Scene->setCurrentFocusItem(item);
//    m_Scene->addItem(item);
//    QFocusEvent event(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
//    QApplication::sendEvent(m_Scene, &event);
//    m_Scene->clear();
//}

////focusOutDeal
//TEST_F(test_graphicsscene, focusOutDeal)
//{
//    QFocusEvent event(QEvent::FocusOut, Qt::TabFocusReason);
//    m_Scene->setActiveSwitching(true);
//    QApplication::sendEvent(m_Scene, &event);
//}

////focusOutDeal
//TEST_F(test_graphicsscene, focusOutDeal1)
//{
//    QFocusEvent event(QEvent::FocusOut, Qt::TabFocusReason);
//    m_Scene->setActiveSwitching(false);
//    QApplication::sendEvent(m_Scene, &event);
//}

//TEST_F(test_graphicsscene, event)
//{
//    QFocusEvent event(QEvent::FocusOut);
//    m_Scene->event(&event);
//}

////setIsContextMenu
//TEST_F(test_graphicsscene, setIsContextMenu)
//{
//    m_Scene->setIsShowCurrentItem(false);
//}

////currentItemInit
//TEST_F(test_graphicsscene, currentItemInit)
//{
//    CFocusItem *item = new CFocusItem();
//    m_Scene->setCurrentFocusItem(item);
//    m_Scene->addItem(item);
//    m_Scene->currentItemInit();
//    m_Scene->clear();
//}

////getActiveSwitching
//TEST_F(test_graphicsscene, getActiveSwitching)
//{
//    m_Scene->getActiveSwitching();
//}
