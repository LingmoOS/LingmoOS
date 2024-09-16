// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define protected public
#define private public
//#include "cgraphicsview.h"
//#include "pageview.h"
#include "drawboard.h"
#include "pagescene.h"
#include <qaction.h>
#undef protected
#undef private

#include "ccutdialog.h"
#include "ccuttool.h"
#include "csizehandlerect.h"
#include "application.h"
#include "toptoolbar.h"

#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"
#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#if TEST_CUT_ITEM

TEST(CutItem, TestCutItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(CutItem, TestCutItemProperty)
{
    PageView *view = getCurView();

    QRectF originRect = view->page()->pageRect();

    ASSERT_NE(view, nullptr);

    // [2] 裁剪图元需要单独进行处理才可以
    int addedCount = view->pageScene()->allPageItems().count(); //view->drawScene()->getBzItems().count();

    drawApp->setCurrentTool(cut);
    QTest::qWait(300);

    //ASSERT_EQ(view->drawScene()->getBzItems().count(), addedCount + 1);
    //ASSERT_EQ(view->drawScene()->getBzItems().first()->type(), CutType);
    ASSERT_EQ(view->pageScene()->allPageItems().count(), addedCount + 1);
    ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), CutType);

    // 获取确认裁剪按钮
    QPushButton  *cutDoneBtn = drawApp->topMainWindow()->findChild<QPushButton *>("Cut done pushbutton");

    ASSERT_NE(cutDoneBtn, nullptr);

    // [2.1] 手动设置裁剪大小(自由模式)
    DLineEdit *widthLineEdit = drawApp->topMainWindow()->findChild<DLineEdit *>("CutWidthLineEdit");
    ASSERT_NE(widthLineEdit, nullptr);
    widthLineEdit->setText("1000");
    emit widthLineEdit->editingFinished();
    emit cutDoneBtn->clicked();
    QTest::qWait(100);
    ASSERT_EQ(view->pageScene()->sceneRect().width(), widthLineEdit->text().toInt());

    drawApp->setCurrentTool(cut);
    QTest::qWait(100);

    auto topToolBar = drawApp->topToolbar();
    DLineEdit *heightLineEdit = topToolBar->findChild<DLineEdit *>("CutHeightLineEdit");
    ASSERT_NE(heightLineEdit, nullptr);
    heightLineEdit->setText("500");
    emit heightLineEdit->editingFinished();
    emit cutDoneBtn->clicked();
    QTest::qWait(100);
    ASSERT_EQ(view->pageScene()->sceneRect().height(), heightLineEdit->text().toInt());

    // [2.2] 1:1 模式
    drawApp->setCurrentTool(cut);
    QTest::qWait(100);
    QPushButton *btn = topToolBar->findChild<QPushButton *>("Cut ratio(1:1) pushbutton");
    ASSERT_NE(btn, nullptr);
    btn->toggle();
    emit cutDoneBtn->clicked();
    QTest::qWait(100);
    ASSERT_EQ(view->pageScene()->sceneRect().width(), widthLineEdit->text().toInt());
    ASSERT_EQ(view->pageScene()->sceneRect().height(), heightLineEdit->text().toInt());

    // [2.3] 2:3 模式
    drawApp->setCurrentTool(cut);
    QTest::qWait(100);
    btn = topToolBar->findChild<QPushButton *>("Cut ratio(2:3) pushbutton");
    ASSERT_NE(btn, nullptr);
    btn->toggle();
    emit cutDoneBtn->clicked();
    QTest::qWait(100);
    ASSERT_EQ(int(view->pageScene()->sceneRect().width()), widthLineEdit->text().toInt());
    ASSERT_EQ(view->pageScene()->sceneRect().height(), heightLineEdit->text().toInt());

    // [2.4] 8:5 模式
    drawApp->setCurrentTool(cut);
    QTest::qWait(100);
    btn = topToolBar->findChild<QPushButton *>("Cut ratio(8:5) pushbutton");
    ASSERT_NE(btn, nullptr);
    btn->toggle();
    emit cutDoneBtn->clicked();
    QTest::qWait(100);
    ASSERT_EQ(qRound(view->pageScene()->sceneRect().width()), widthLineEdit->text().toInt());
    ASSERT_EQ(qRound(view->pageScene()->sceneRect().height()), heightLineEdit->text().toInt());

    // [2.5] 16:9 模式
    drawApp->setCurrentTool(cut);
    QTest::qWait(100);
    btn = topToolBar->findChild<QPushButton *>("Cut ratio(16:9) pushbutton");
    ASSERT_NE(btn, nullptr);
    btn->toggle();
    emit cutDoneBtn->clicked();
    QTest::qWait(100);
    ASSERT_EQ(qRound(view->pageScene()->sceneRect().width()), widthLineEdit->text().toInt());
    ASSERT_EQ(qRound(view->pageScene()->sceneRect().height()), heightLineEdit->text().toInt());

    // [2.6] 原始恢复
    drawApp->setCurrentTool(cut);
    QTest::qWait(100);
    btn = topToolBar->findChild<QPushButton *>("Cut ratio(Original) pushbutton");
    ASSERT_NE(btn, nullptr);
    view->pageScene()->setSceneRect(QRectF(0, 0, 400, 400));
    btn->toggle();
    emit cutDoneBtn->clicked();
    QTest::qWait(100);
    ASSERT_EQ(view->pageScene()->sceneRect().width(), originRect.width());
    ASSERT_EQ(view->pageScene()->sceneRect().height(), originRect.height());
}

TEST(CutItem, TestResizeCutItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    drawApp->setCurrentTool(cut);
    QTest::qWait(100);

    CCutTool *pTool = qobject_cast<CCutTool *>(drawApp->drawBoard()->currentTool_p());
    ASSERT_NE(pTool, nullptr);

    // cutAttriWidget  = drawApp->topToolbar()->findChild<CCutWidget *>("scene cut attribution widget");
    auto fDoOperate = [ = ]() {
        Handles handles = pTool->getCutItem(view->pageScene())->handleNodes();
        for (int i = 0; i < handles.size(); ++i) {
            HandleNode *pNode = handles[i];
            QPoint startPosInView    = view->mapFromScene(pNode->mapToScene(pNode->boundingRect().center()));
            QPoint desPosInView      =  startPosInView + QPoint(50, 50);
            DTestEventList e;
            e.addMouseMove(startPosInView, 100);
            e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, startPosInView, 100);
            e.addMouseMove(desPosInView, 100);
            e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, desPosInView, 100);

            e.simulate(view->viewport());

            e.clear();

            e.addMouseMove(desPosInView, 100);
            e.addMousePress(Qt::LeftButton, Qt::ShiftModifier, desPosInView, 100);
            e.addMouseMove(startPosInView, 100);
            e.addMouseRelease(Qt::LeftButton, Qt::ShiftModifier, startPosInView, 100);

            e.simulate(view->viewport());
        }
    };

    auto cutItem = pTool->getCutItem(view->pageScene());
    auto size    = view->pageScene()->sceneRect().size().toSize();
    for (int i = cut_1_1; i < cut_count; ++i) {
        //cutAttriWidget->setCutType(ECutType(i));
        QTest::qWait(100);
        fDoOperate();
        QTest::qWait(100);
        //ASSERT_EQ(cutItem->rect().size().toSize(), cutAttriWidget->cutSize());
    }
    //cutAttriWidget->setCutType(cut_free);
    //cutAttriWidget->setCutSize(QSize(10, 10));
    ASSERT_EQ(cutItem->rect().size().toSize(), QSize(10, 10));
}

TEST(CutItem, TestSaveCutItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString CutItemPath = QApplication::applicationDirPath() + "/test_cut.ddf";
    c->setFile(CutItemPath);
    setQuitDialogResult(CCutDialog::Save);
    c->save();

    setQuitDialogResult(2);
    view->page()->close();

    setQuitDialogResult(-1);
    QFileInfo info(CutItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(CutItem, TestOpenCutItemFromFile)
{
    PageView *view = getCurView();

    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString CutItemPath = QApplication::applicationDirPath() + "/test_cut.ddf";

    view->page()->borad()->load(CutItemPath);

    qMyWaitFor([ = ]() {return (view != getCurView());});

    view = getCurView();

    ASSERT_NE(view, nullptr);

    view->page()->close(true);
}

#endif
