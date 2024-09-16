// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include "toptoolbar.h"
#include "application.h"

#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"

#include <QDebug>
#include <DLineEdit>

#include "../testItems/publicApi.h"

#if TEST_SCANLE_ITEM

TEST(ScanleScence, TestScanleScenceCreateView)
{
    createNewViewByShortcutKey();
}

TEST(ScanleScence, TestScanleScence)
{
    MainWindow *w = getMainWindow();
    ASSERT_NE(w, nullptr);
    TopTilte *toptoolbar = w->topTitle();
    ASSERT_NE(toptoolbar, nullptr);
    /**
     * @brief 缩放view以及缩放菜单测试
     */
    DIconButton *fbtn;
    fbtn = toptoolbar->findChild<DIconButton *>("Zoom reduce button");
    ASSERT_NE(fbtn, nullptr);
    fbtn->clicked();
    fbtn->clicked();

    fbtn = toptoolbar->findChild<DIconButton *>("Zoom increase button");
    ASSERT_NE(fbtn, nullptr);
    fbtn->clicked();
    fbtn->clicked();

    DZoomMenuComboBox *box = toptoolbar->findChild<DZoomMenuComboBox *>("zoomMenuComboBox");
    ASSERT_NE(box, nullptr);
    box->setCurrentText("75%");

    DTestEventList e;
    e.addKeyClick(Qt::Key_Minus, Qt::ControlModifier, 100);
    e.addKeyClick(Qt::Key_Minus, Qt::ControlModifier, 100);
    e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
    e.addKeyClick(Qt::Key_Equal, Qt::ControlModifier, 100);
    e.addKeyClick(Qt::Key_Equal, Qt::ControlModifier, 100);
    e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
    e.addKeyClick(Qt::Key_Plus, Qt::ControlModifier, 100);
    e.simulate(getCurView());
}

TEST(ScanleScence, TestSaveScanleScenceToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString ScanleScencePath = QApplication::applicationDirPath() + "/test_scanle.ddf";
    c->setFile(ScanleScencePath);
    c->save();
    c->context()->setDirty(false);
    c->close(true);
}

#if 0
TEST(ScanleScence, TestOpenScanleScenceFromFile)
{
    CGraphicsView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString ScanleScencePath = QApplication::applicationDirPath() + "/test_scanle.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(ScanleScencePath));
    mimedata.setUrls(li);

    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);

    qMyWaitFor([ = ]() {return (view != getCurView());});

    view = getCurView();
    ASSERT_NE(view, nullptr);
    int addedCount = view->drawScene()->getBzItems(view->drawScene()->items()).count();
    ASSERT_EQ(true, addedCount == 0 ? true : false);
}
#endif

#endif
