// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#define protected public
#define private public
#include <qaction.h>

#include "toptoolbar.h"
#include "application.h"

#include "ccuttool.h"
#include <DFloatingButton>
#include <DComboBox>
#include <dzoommenucombobox.h>
#include "cspinbox.h"

#include <QDebug>
#include <DLineEdit>

#include "publicApi.h"

#undef protected
#undef private

#if TEST_PICTURE_ITEM

TEST(PictureItem, TestPictureItemCreateView)
{
    createNewViewByShortcutKey();
}

TEST(PictureItem, TestDrawPictureItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    int addedCount = view->pageScene()->allPageItems().count();
    QString path = QApplication::applicationDirPath() + "/test.png";
    QPixmap pix(":/test.png");
    ASSERT_EQ(true, pix.save(path, "PNG"));
    QTest::qWait(300);

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(path));
    mimedata.setUrls(li);

    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);
    QTest::qWait(300);

    ASSERT_EQ(view->pageScene()->allPageItems().count(), addedCount + 1);
    //ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), DyLayer);
}

TEST(PictureItem, TestBlurPictureItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    Page *c = getMainWindow()->drawBoard()->currentPage();

    ASSERT_NE(c, nullptr);

    //ASSERT_EQ(view->pageScene()->allPageItems().first()->type(), DyLayer);

    //auto pPictureItem = dynamic_cast<JDynamicLayer *>(view->pageScene()->allPageItems().first());

    //view->pageScene()->selectPageItem(pPictureItem);

    //1.切换到模糊工具下
    drawApp->setCurrentTool(blur);
    QTest::qWait(200);

    //2.设置模糊参数
//    BlurWidget *pBLurAttriWidget = drawApp->topToolbar()->findChild<BlurWidget *>("BlurWidget");

//    ASSERT_NE(pBLurAttriWidget, nullptr);

//    pBLurAttriWidget->setBlurType(BlurEffect);
//    QTest::qWait(100);

//    // Blur width
//    pBLurAttriWidget->setBlurWidth(30);
//    QTest::qWait(1000);

//    //3.对图片进行模糊
//    QPoint picturePointInGloble = view->mapFromScene(pPictureItem->sceneBoundingRect().topLeft());
//    QSize pictureSize = pPictureItem->rect().size().toSize();
//    QRect rectInView(picturePointInGloble, pictureSize);

//    DTestEventList blurEvent;
//    blurEvent.clear();
//    blurEvent.addMouseMove(rectInView.topLeft(), 200);

//    blurEvent.addMousePress(Qt::LeftButton, Qt::NoModifier, rectInView.topLeft(), 200);
//    blurEvent.addMouseMove(rectInView.center(), 200);
//    blurEvent.addMouseMove(rectInView.bottomRight(), 200);

//    blurEvent.addMouseRelease(Qt::LeftButton, Qt::NoModifier, rectInView.bottomRight(), 200);
//    blurEvent.simulate(view->viewport());

//    //ASSERT_EQ(pPictureItem->_blurInfos.count(), 1);

//    //切换模糊类型再测试一次
//    {
//        // Blur Masic Type测试马赛克式模糊
//        pBLurAttriWidget->setBlurType(MasicoEffect);
//        QTest::qWait(100);

//        DTestEventList blurEvent;
//        blurEvent.clear();
//        blurEvent.addMouseMove(rectInView.bottomLeft(), 200);

//        blurEvent.addMousePress(Qt::LeftButton, Qt::NoModifier, rectInView.bottomLeft(), 200);
//        blurEvent.addMouseMove(rectInView.center(), 200);
//        blurEvent.addMouseMove(rectInView.topRight(), 200);

//        blurEvent.addMouseRelease(Qt::LeftButton, Qt::NoModifier, rectInView.topRight(), 200);
//        blurEvent.addMouseClick(Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), 200);
//        blurEvent.simulate(view->viewport());

//        //ASSERT_EQ(pPictureItem->_blurInfos.count(), 2);
//    }

//    //橡皮擦
//    ASSERT_EQ(getToolButtonStatus(eraser), true);

//    DTestEventList e;
//    e.clear();
//    e.addKeyClick(Qt::Key_E, Qt::NoModifier, 200);

//    //hover
//    e.addMouseMove(rectInView.topLeft());
//    e.addDelay(500);

//    //clear
//    e.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, rectInView.topLeft(), 200);
//    e.addMouseMove(rectInView.center(), 200);
//    e.addMouseRelease(Qt::MouseButton::LeftButton);
//    e.addDelay(200);
//    e.simulate(view->viewport());

//    /*e.clear();
//    e.addKeyClick(Qt::Key_Z, Qt::ControlModifier);
//    e.addDelay(200);
//    e.addKeyClick(Qt::Key_Y, Qt::ControlModifier);
//    e.addDelay(200);
//    e.simulate(view->viewport());*/

//    view->pageScene()->selectPageItem(pPictureItem);
}


TEST(PictureItem, TestCopyPictureItem)
{
    keyShortCutCopyItem();

    PageView *view = getCurView();

    ASSERT_NE(view, nullptr);

    auto allItems = view->pageScene()->allPageItems();

    ASSERT_EQ(allItems.count(), 2);
}

TEST(PictureItem, TestPictureItemProperty)
{
    PageView *view = getCurView();

    ASSERT_NE(view, nullptr);

    auto allItems = view->pageScene()->allPageItems();

    ASSERT_EQ(allItems.count(), 2);

    view->pageScene()->clearSelections();

    view->pageScene()->selectPageItem(allItems.last());

    auto selectionsItems0 = view->pageScene()->selectedPageItems();

    ASSERT_EQ(selectionsItems0.count(), 1);

    //ASSERT_EQ(selectionsItems0.first()->type(), /*PictureType*/DyLayer);

    auto fDoOperate = [ = ]() {
        // 左旋转
        QPushButton *btn = drawApp->topToolbar()->findChild<QPushButton *>("PicLeftRotateBtn");
        ASSERT_NE(btn, nullptr);
        emit btn->clicked();
        QTest::qWait(200);

        // 右旋转
        btn = drawApp->topToolbar()->findChild<QPushButton *>("PicRightRotateBtn");
        ASSERT_NE(btn, nullptr);
        emit btn->clicked();
        QTest::qWait(200);

        // 水平翻转
        btn = drawApp->topToolbar()->findChild<QPushButton *>("PicFlipHBtn");
        ASSERT_NE(btn, nullptr);
        emit btn->clicked();
        QTest::qWait(200);

        // 水平翻转
        btn = drawApp->topToolbar()->findChild<QPushButton *>("PicFlipVBtn");
        ASSERT_NE(btn, nullptr);
        emit btn->clicked();
        QTest::qWait(200);

        // 自适应scence
        btn = drawApp->topToolbar()->findChild<QPushButton *>("PicFlipAdjustmentBtn");
        ASSERT_NE(btn, nullptr);
        emit btn->clicked();
        QTest::qWait(200);
    };

    fDoOperate();

    view->pageScene()->clearSelections();

    view->pageScene()->selectPageItem(allItems.first());

    view->pageScene()->selectPageItem(allItems.last());

    auto selectionsItems1 = view->pageScene()->selectedPageItems();

    ASSERT_EQ(selectionsItems1.count(), 2);

    fDoOperate();
}

TEST(PictureItem, TestPictureItemPenetrable)
{
    PageView *view = getCurView();

    ASSERT_NE(view, nullptr);

    auto allItems = view->pageScene()->allPageItems();

    ASSERT_EQ(allItems.count(), 2);

    //allItems.first()->rasterToSelfLayer(false);

    allItems = view->pageScene()->allPageItems();

    ASSERT_EQ(allItems.count(), 2);

    auto pictureItem = dynamic_cast</*CPictureItem*/RasterItem *>(allItems.first());

    ASSERT_NE(pictureItem, nullptr);

    ASSERT_EQ(pictureItem->isPosPenetrable(QPointF(0, 0)), false);
    ASSERT_EQ(pictureItem->isRectPenetrable(QRectF()), false);

    //ASSERT_EQ(pictureItem->rasterSelf().isNull(), false);
}

TEST(PictureItem, TestResizePictureItem)
{
    resizeItem();
}

TEST(PictureItem, TestSelectAllPictureItem)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 全选图元
    DTestEventList e;
    e.addMouseMove(QPoint(20, 20), 100);
    e.addMousePress(Qt::LeftButton, Qt::NoModifier, QPoint(10, 10), 100);
    e.addMouseMove(QPoint(1800, 900), 100);
    e.addMouseRelease(Qt::LeftButton, Qt::NoModifier, QPoint(1000, 1000), 100);
    e.addKeyPress(Qt::Key_A, Qt::ControlModifier, 100);
    e.addKeyRelease(Qt::Key_A, Qt::ControlModifier, 100);
    e.simulate(view->viewport());

    ASSERT_EQ(getToolButtonStatus(eraser), false);

    // 水平等间距对齐
    //view->m_itemsVEqulSpaceAlign->triggered(true);
    // 垂直等间距对齐
    //view->m_itemsHEqulSpaceAlign->triggered(true);

    //滚轮事件
    QWheelEvent wheelevent(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ControlModifier);
    view->wheelEvent(&wheelevent);
    QWheelEvent wheelevent2(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
    view->wheelEvent(&wheelevent2);
    QWheelEvent wheelevent3(QPointF(1000, 1000), 100, Qt::MouseButton::NoButton, Qt::KeyboardModifier::ShiftModifier);
    view->wheelEvent(&wheelevent3);
}

TEST(PictureItem, TestSavePictureItemToFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    // save ddf file
    QString PictureItemPath = QApplication::applicationDirPath() + "/test_picture.ddf";
    c->setFile(PictureItemPath);
    c->save();
    c->close(true);

    QFileInfo info(PictureItemPath);
    ASSERT_TRUE(info.exists());
}

TEST(PictureItem, TestOpenPictureItemFromFile)
{
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);

    // 打开保存绘制的 ddf
    QString PictureItemPath = QApplication::applicationDirPath() + "/test_picture.ddf";

    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(PictureItemPath));
    mimedata.setUrls(li);

    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);
    qMyWaitFor([ = ]() {
        return (view != getCurView() && getCurView()->pageScene()->allPageItems().count());
    });

    view = getCurView();
    ASSERT_NE(view, nullptr);
    int addedCount = view->pageScene()->allPageItems().count();
    ASSERT_EQ(addedCount, 2);
    view->page()->close(true);
}

TEST(PictureItem, TestOldPictureItem)
{
    createNewViewByShortcutKey();

    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    {
        RasterItem *oldItem = new RasterItem;
        EXPECT_NE(oldItem, nullptr);
        oldItem->setAttributionVar(EImageLeftRot, QVariant(), 0);
        oldItem->setAttributionVar(EImageRightRot, QVariant(), 0);
        oldItem->setAttributionVar(EImageHorFilp, QVariant(), 0);
        oldItem->setAttributionVar(EImageVerFilp, QVariant(), 0);
        oldItem->setAttributionVar(EImageAdaptScene, QVariant(), 0);
        delete oldItem;
    }
    {
//        RasterItem *oldItem = new RasterItem(QRectF(0, 0, 400, 300), QPixmap(":/test.png"));
//        EXPECT_NE(oldItem, nullptr);
//        c->scene()->addCItem(oldItem);
//        oldItem->setPixmap(QPixmap(":/test.png"));
//        EXPECT_EQ(oldItem->pixmap().isNull(), false);
//        oldItem->updateBlurPixmap();
//        oldItem->setAttributionVar(EImageLeftRot, QVariant(), 0);
//        oldItem->setAttributionVar(EImageRightRot, QVariant(), 0);
//        oldItem->setAttributionVar(EImageHorFilp, QVariant(), 0);
//        oldItem->setAttributionVar(EImageVerFilp, QVariant(), 0);

//        EXPECT_EQ(oldItem->isPosPenetrable(QPointF(0, 0)), false);

//        auto unit = oldItem->getGraphicsUnit(EDuplicate);
//        EXPECT_NE(unit.data.pPic, nullptr);
//        oldItem->loadGraphicsUnit(unit);
//        unit.release();

//        c->scene()->removeCItem(oldItem);

//        delete oldItem;
    }
    c->close(true);
}

TEST(PictureItem, TestImportPictures)
{
    createNewViewByShortcutKey();
    PageView *view = getCurView();
    ASSERT_NE(view, nullptr);
    Page *c = getMainWindow()->drawBoard()->currentPage();
    ASSERT_NE(c, nullptr);

    /*QMimeData mimedata;
    QList<QUrl> li;
    QString path;
    for (int i = 0; i < 5; ++i) {
        QString fileName = QString("/test_%1.jpg").arg(i + 1);
        QPixmap pix(":" + fileName);
        path = QApplication::applicationDirPath() + fileName;
        ASSERT_EQ(true, pix.save(path, "jpg"));
        QTest::qWait(300);

        li.append(QUrl(path));
    }

    mimedata.setUrls(li);

    setQuitDialogResult(1);
    const QPoint pos = view->viewport()->rect().center();
    QDragEnterEvent eEnter(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &eEnter);

    QDropEvent e(pos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    dApp->sendEvent(view->viewport(), &e);
    QTest::qWait(3000);
    view->page()->close(true);*/

    QString path;
    QStringList pathList;
    for (int i = 0; i < 5; ++i) {
        QString fileName = QString("/test_%1.jpg").arg(i + 1);
        QPixmap pix(":" + fileName);
        path = QApplication::applicationDirPath() + fileName;
        ASSERT_EQ(true, pix.save(path, "jpg"));
        pathList << path;
    }

    setQuitListResults(QList<int>() << 1 << 1 << 0 << 0 << -1);
    getMainWindow()->drawBoard()->loadFiles(pathList, false);

    QTest::qWait(1000);
    view->page()->close(true);
}

#endif
