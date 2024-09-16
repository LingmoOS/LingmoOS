// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include <DGuiApplicationHelper>
#include "imageviewer.h"
#include "imageengine.h"
#include "service/imagedataservice.h"
#include "viewpanel/navigationwidget.h"
#define  private public
#define protected public
#include "viewpanel/viewpanel.h"


//view panel
TEST_F(gtestview, LibViewPanel)
{
    //初始化
    QWidget *widget = new QWidget();
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
    panel->loadImage(QApplication::applicationDirPath() + "/gif.gif", {QApplication::applicationDirPath() + "/gif.gif",
                                                                       QApplication::applicationDirPath() + "/tif.tif",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/svg.svg"
                                                                      });
    panel->initFloatingComponent();
    panel->show();

    //键盘与鼠标事件

    QTest::keyClick(panel, Qt::Key_Right, Qt::NoModifier, 200);
    //动态图
    QTest::keyClick(panel, Qt::Key_Right, Qt::KeyboardModifier::NoModifier, 200);
    QTest::keyClick(panel, Qt::Key_Left, Qt::KeyboardModifier::NoModifier, 200);
    QTest::keyClick(panel, Qt::Key_Up, Qt::KeyboardModifier::NoModifier, 200);
    QTest::keyClick(panel, Qt::Key_Plus, Qt::KeyboardModifier::ControlModifier, 200);
    QTest::keyClick(panel, Qt::Key_0, Qt::KeyboardModifier::ControlModifier, 200);
    QTest::keyClick(panel, Qt::Key_Minus, Qt::KeyboardModifier::ControlModifier, 200);
    QTest::keyClick(panel, Qt::Key_Down, Qt::KeyboardModifier::NoModifier, 200);
    QTest::keyClick(panel, Qt::Key_Escape, Qt::KeyboardModifier::NoModifier, 200);


    //多页图
    QTest::keyClick(panel, Qt::Key_Right, Qt::KeyboardModifier::NoModifier, 200);
    QTest::keyClick(panel, Qt::Key_Up, Qt::KeyboardModifier::NoModifier, 200);
    QTest::keyClick(panel, Qt::Key_Plus, Qt::KeyboardModifier::ControlModifier, 200);
    QTest::keyClick(panel, Qt::Key_0, Qt::KeyboardModifier::ControlModifier, 200);
    QTest::keyClick(panel, Qt::Key_Minus, Qt::KeyboardModifier::ControlModifier, 200);
    QTest::keyClick(panel, Qt::Key_Down, Qt::KeyboardModifier::NoModifier, 200);
    QTest::keyClick(panel, Qt::Key_Escape, Qt::KeyboardModifier::NoModifier, 200);


    panel->deleteLater();
    panel = nullptr;
    QTest::qWait(500);

    EXPECT_EQ(true, QFileInfo(QApplication::applicationDirPath() + "/gif.gif").isFile());

}
//view panel
TEST_F(gtestview, LibViewPanel1)
{
    //初始化
    LibViewPanel *panel = new LibViewPanel();
    panel->loadImage(QApplication::applicationDirPath() + "/gif.gif", {QApplication::applicationDirPath() + "/gif.gif",
                                                                       QApplication::applicationDirPath() + "/tif.tif",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/svg.svg"
                                                                      });
    panel->initFloatingComponent();
    panel->show();
//    //基本函数遍历
    panel->toggleFullScreen();
    panel->slotBottomMove();
    panel->toggleFullScreen();
    panel->slotBottomMove();

    panel->deleteLater();
    panel = nullptr;
    QTest::qWait(500);

    EXPECT_EQ(true, QFileInfo(QApplication::applicationDirPath() + "/gif.gif").isFile());
}
TEST_F(gtestview, LibViewPanel_ocr)
{
    QWidget *widget = new QWidget();
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
    panel->loadImage(QApplication::applicationDirPath() + "/gif.gif", {QApplication::applicationDirPath() + "/gif.gif",
                                                                       QApplication::applicationDirPath() + "/tif.tif",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/svg.svg"
                                                                      });
    panel->getBottomtoolbarButton(imageViewerSpace::ButtonType::ButtonTypeOcr);
    panel->slotOcrPicture();
    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;

    EXPECT_EQ(true, QFileInfo(QApplication::applicationDirPath() + "/gif.gif").isFile());
}
TEST_F(gtestview, LibViewPanel_startChooseFileDialog1)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
//    panel->backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel->initSlidePanel();
    panel->resetBottomToolbarGeometry(true);
    panel->resetBottomToolbarGeometry(false);
    panel->slotRotateImage(90);
    QTest::qWait(1000);
    panel->slotRotateImage(-90);
    QTest::qWait(1000);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_noAnimationBottomMove)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
//    panel->backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel->noAnimationBottomMove();
    QTest::qWait(500);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_slotBottomMove)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
//    panel->backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel->slotBottomMove();
    QTest::qWait(500);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_setWallPaper_image)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
    QImage img(QApplication::applicationDirPath() + "/svg.svg");
    panel->setWallpaper(img);
    QTest::qWait(500);
    QImage img1(QApplication::applicationDirPath() + "/test/jpg100.jpg");
    panel->setWallpaper(img1);
    QTest::qWait(500);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_setWallPaper_path)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
    panel->setWallpaper(QApplication::applicationDirPath() + "/svg.svg");
    QTest::qWait(500);
    panel->setWallpaper(QApplication::applicationDirPath() + "/test/jpg100.jpg");
    QTest::qWait(500);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_mimeDataDrag)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(QApplication::applicationDirPath() + "/test/jpg.jpg"));
    mimedata.setUrls(li);
    panel->resize(800, 600);

    auto dropPos = panel->rect().center();
    QDragEnterEvent eEnter(dropPos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    panel->dragEnterEvent(&eEnter);

    QDragMoveEvent emove(dropPos + QPoint(10, 10), Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
    panel->dragMoveEvent(&emove);

//    QDropEvent e(dropPos, Qt::IgnoreAction, &mimedata, Qt::LeftButton, Qt::NoModifier);
//    panel->dropEvent(&e);

    QTest::qWait(3000);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_leaveEvent)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
    QMimeData mimedata;
    QList<QUrl> li;
    li.append(QUrl(QApplication::applicationDirPath() + "/test/jpg.jpg"));
    mimedata.setUrls(li);
    panel->resize(800, 600);

    QEvent event(QEvent::Leave);
    panel->leaveEvent(&event);

    QTest::qWait(100);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}


TEST_F(gtestview, LibViewPanel_showTopBottom)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
//    panel->backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel->showTopBottom();
    QTest::qWait(100);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_showAnimationTopBottom)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
//    panel->backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel->showAnimationTopBottom();
    QTest::qWait(100);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}


TEST_F(gtestview, LibViewPanel_hideTopBottom)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
//    panel->backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel->hideTopBottom();
    QTest::qWait(100);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_hideAnimationTopBottom)
{
    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);
//    panel->backImageView(QApplication::applicationDirPath() + "/svg.svg");
    panel->hideAnimationTopBottom();
    QTest::qWait(100);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;
}

TEST_F(gtestview, LibViewPanel_Menu)
{
//    //键盘与鼠标事件
//    QTestEventList e;


    QWidget *widget = new QWidget();
    //初始化
    LibViewPanel *panel = new LibViewPanel(nullptr, widget);

    panel->loadImage(QApplication::applicationDirPath() + "/gif.gif", {QApplication::applicationDirPath() + "/gif.gif",
                                                                       QApplication::applicationDirPath() + "/tif.tif",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/jpg.jpg",
                                                                       QApplication::applicationDirPath() + "/svg.svg"
                                                                      });
    //菜单
    //还剩IdPrint会崩
    QAction menuAction;

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdFullScreen);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdSetAsWallpaper);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(10000);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdRename);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdStartSlideShow);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(2000);

    //add new
    QTest::keyClick(panel, Qt::Key_Escape, Qt::KeyboardModifier::NoModifier, 200);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdCopy);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdShowNavigationWindow);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdHideNavigationWindow);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
    QTest::qWait(500);
    DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
    QTest::qWait(500);


    QTest::keyClick(panel, Qt::Key_Escape, Qt::KeyboardModifier::NoModifier, 200);
    QTest::qWait(400);

    //add new
//    QTest::mousePress(panel, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 300);
//    QTest::mouseMove(panel, QPoint(20, 20), 300);
//    QTest::mouseRelease(panel, Qt::LeftButton, Qt::NoModifier, QPoint(200, 1020), 300);
//    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdRotateClockwise);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdRotateCounterclockwise);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);


    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdDisplayInFileManager);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdImageInfo);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdOcr);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(500);

    menuAction.setProperty("MenuID", imageViewerSpace::NormalMenuItemId::IdMoveToTrash);
    panel->onMenuItemClicked(&menuAction);
    QTest::qWait(400);

//    //ImageGraphicsView
//    panel->loadImage("", {});

//    auto view = panel->m_view;
//    view->clear();
    panel->m_view->setImage(QApplication::applicationDirPath() + "/svg2.svg", QImage());

    QTest::qWait(1000);

    panel->deleteLater();
    panel = nullptr;
    widget->deleteLater();
    widget = nullptr;

}



