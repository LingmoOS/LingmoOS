// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtestview.h"
#define  private public
#include <QKeySequence>
#include <QShortcut>
#include <QEnterEvent>
#include <QFile>
#include <QDir>
#include <DApplication>

#include "accessibility/ac-desktop-define.h"
#include "imageviewer.h"

#include "widgets/extensionpanel.h"
#include "widgets/formlabel.h"
#include "widgets/imagebutton.h"
#include "widgets/printhelper.h"
#include "widgets/renamedialog.h"
#include "widgets/toast.h"
#include "widgets/toptoolbar.h"

#define  private public
#include "viewpanel/viewpanel.h"
#include "viewpanel/navigationwidget.h"
#include "slideshow/slideshowpanel.h"
#include "service/imagedataservice.h"
#include "viewpanel/contents/bottomtoolbar.h"
#include "viewpanel/contents/imgviewwidget.h"
#include "unionimage/baseutils.h"
#include "unionimage/imageutils.h"
#include "unionimage/imgoperate.h"
#include "unionimage/snifferimageformat.h"
#include "unionimage/unionimage.h"

gtestview::gtestview()
{

}
TEST_F(gtestview, cpFile)
{
    QFile::copy(":/500Kavi.avi", QApplication::applicationDirPath() + "/500Kavi.avi");
    QFile(QApplication::applicationDirPath() + "/500Kavi.avi").setPermissions(\
                                                                              QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                              QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/gif.gif", QApplication::applicationDirPath() + "/gif.gif");
    QFile(QApplication::applicationDirPath() + "/gif.gif").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/gif2.gif", QApplication::applicationDirPath() + "/gif2.gif");
    QFile(QApplication::applicationDirPath() + "/gif2.gif").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/ico.ico", QApplication::applicationDirPath() + "/ico.ico");
    QFile(QApplication::applicationDirPath() + "/ico.ico").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/jpg.jpg", QApplication::applicationDirPath() + "/jpg.jpg");
    QFile(QApplication::applicationDirPath() + "/jpg.jpg").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/mng.mng", QApplication::applicationDirPath() + "/mng.mng");
    QFile(QApplication::applicationDirPath() + "/mng.mng").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/png.png", QApplication::applicationDirPath() + "/png.png");
    QFile(QApplication::applicationDirPath() + "/png.png").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg.svg", QApplication::applicationDirPath() + "/svg.svg");
    QFile(QApplication::applicationDirPath() + "/svg.svg").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg1.svg", QApplication::applicationDirPath() + "/svg1.svg");
    QFile(QApplication::applicationDirPath() + "/svg1.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg2.svg", QApplication::applicationDirPath() + "/svg2.svg");
    QFile(QApplication::applicationDirPath() + "/svg2.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/svg1.svg", QApplication::applicationDirPath() + "/svg3.svg");
    QFile(QApplication::applicationDirPath() + "/svg3.svg").setPermissions(\
                                                                           QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                           QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/tif.tif", QApplication::applicationDirPath() + "/tif.tif");
    QFile(QApplication::applicationDirPath() + "/tif.tif").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/wbmp.wbmp", QApplication::applicationDirPath() + "/wbmp.wbmp");
    QFile(QApplication::applicationDirPath() + "/wbmp.wbmp").setPermissions(\
                                                                            QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                            QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/dds.dds", QApplication::applicationDirPath() + "/dds.dds");
    QFile(QApplication::applicationDirPath() + "/dds.dds").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/tga.tga", QApplication::applicationDirPath() + "/tga.tga");
    QFile(QApplication::applicationDirPath() + "/tga.tga").setPermissions(\
                                                                          QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                          QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QFile::copy(":/errorPic.icns", QApplication::applicationDirPath() + "/errorPic.icns");
    QFile(QApplication::applicationDirPath() + "/errorPic.icns").setPermissions(\
                                                                                QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                                QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    QDir a(QApplication::applicationDirPath());
    a.mkdir("test");
    QFile::copy(":/jpg.jpg", QApplication::applicationDirPath() + "/test/jpg.jpg");
    QFile(QApplication::applicationDirPath() + "/test/jpg.jpg").setPermissions(\
                                                                               QFile::WriteUser | QFile::ReadUser | QFile::WriteOther | \
                                                                               QFile::ReadOther | QFile::ReadGroup | QFile::WriteGroup);

    EXPECT_EQ(true, QFileInfo(QApplication::applicationDirPath() + "/errorPic.icns").isFile());
}

//主窗体
TEST_F(gtestview, MainWindow)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr);
    QStringList paths{QApplication::applicationDirPath() + "/svg.svg", QApplication::applicationDirPath() + "/test/jpg.jpg"};
    m_imageViewer->startChooseFileDialog();
    bool bRet = m_imageViewer->startdragImage({});
    bRet = m_imageViewer->startdragImage({"smb-share:server=bisuhfawe.png"});
    bRet = m_imageViewer->startdragImage({"mtp:host=bisuhfawe.png"});
    bRet = m_imageViewer->startdragImage({"gphoto2:host=bisuhfawe.png"});
    bRet = m_imageViewer->startdragImage({"gphoto2:host=Apple=bisuhfawe.png"});
    bRet = m_imageViewer->startdragImage({QDir::homePath() + "/.local/share/Trash"});
    bRet = m_imageViewer->startdragImage(paths);
    bRet = m_imageViewer->startdragImageWithUID(paths);
    if (bRet) {
        m_imageViewer->showNormal();
        m_imageViewer->resize(800, 600);
        m_imageViewer->resize(1000, 500);

        m_imageViewer->setTopBarVisible(false);
        m_imageViewer->setTopBarVisible(true);
        m_imageViewer->setBottomtoolbarVisible(false);
        m_imageViewer->setTopBarVisible(true);

        QTest::qWait(500);
    }

    m_imageViewer->deleteLater();
    QTest::qWait(500);

    //另外一种使用方式
    m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr);
    paths = QStringList{QApplication::applicationDirPath() + "/tif.tif"};
    m_imageViewer->startImgView(paths[0], paths);
    QTest::qWait(500);
    m_imageViewer->deleteLater();
    QTest::qWait(500);

    EXPECT_EQ(true, bRet);
}

//widgets
TEST_F(gtestview, Widgets)
{
    //extension panel
    {
        ExtensionPanel panel;
        panel.updateRectWithContent(100);
        QWidget x;
        panel.setContent(&x);

        QTest::qWait(200);

        panel.show();
        QTestEventList e1;
        e1.addKeyClick(Qt::Key::Key_Escape);
        e1.simulate(&x);
        QTest::qWait(200);

        panel.show();
        QTestEventList e2;
        e2.addKeyClick(Qt::Key::Key_I, Qt::KeyboardModifier::ControlModifier);
        e2.simulate(&panel);
        QTest::qWait(200);
    }

    //form label
    {
        SimpleFormLabel label_1("12345");
        label_1.resize(100, 100);
        label_1.show();

        SimpleFormField label_2;
        label_2.resize(100, 100);
        label_2.show();

        QTest::qWait(200);
    }

    //image button
    {
        ImageButton button;
        button.setDisabled(false);
        button.show();

        QTestEventList e;
        e.addMouseMove(QPoint(1, 1));
        e.addDelay(1500);
        e.addMouseClick(Qt::MouseButton::LeftButton);
        e.addDelay(500);
        e.addMouseMove(QPoint(999, 999));
        e.addDelay(1500);
        e.simulate(&button);

        button.setToolTip("123321");
        e.simulate(&button);

        QTest::qWait(200);
    }

    //print helper
    {
        auto helper = PrintHelper::getIntance();

        helper->showPrintDialog({QApplication::applicationDirPath() + "/jpg.jpg",
                                 QApplication::applicationDirPath() + "/tif.tif"});

        helper->deleteLater();
        QTest::qWait(500);
    }

    //rename dialog
    {
        RenameDialog dialog("12345.jpg");
        dialog.GetFileName();
        dialog.GetFilePath();

        QTestEventList e;
        e.addMouseClick(Qt::MouseButton::LeftButton);
        e.addDelay(400);

        dialog.m_lineedt->setText("321.png");
        dialog.show();
        e.simulate(dialog.okbtn);

        dialog.m_lineedt->setText("");
        dialog.show();
        e.simulate(dialog.cancelbtn);

        /*e.clear();
        e.addMouseClick(Qt::MouseButton::LeftButton);
        e.addDelay(200);
        e.addKeyClick(Qt::Key_5);
        e.addDelay(200);
        e.addKeyClick(Qt::Key_Enter);
        e.addDelay(200);
        dialog.show();
        e.simulate(dialog.m_lineedt);*/

        QTest::qWait(500);
    }

    //toast
    {
        Toast toast;
        toast.text();
        toast.icon();
        toast.setText("123");
        toast.setIcon(QApplication::applicationDirPath() + "/ico.ico");
        toast.setIcon(QIcon(QApplication::applicationDirPath() + "/ico.ico"));
    }

    //top tool bar
    {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
        LibTopToolbar toolBar(false, nullptr);
        toolBar.setTitleBarTransparent(false);

        toolBar.show();
        QTestEventList e;
        e.addMouseDClick(Qt::MouseButton::LeftButton);
        e.addDelay(500);
        e.addMouseDClick(Qt::MouseButton::LeftButton);
        e.addDelay(500);
        e.simulate(&toolBar);

        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
        QTest::qWait(200);
    }
    bool bRet = false;
    if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::LightType) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);
}

TEST_F(gtestview, LibImageDataService)
{
    auto testPath = QApplication::applicationDirPath() + "/jpg.jpg";
    LibImageDataService::instance()->imageIsLoaded(testPath);
    LibImageDataService::instance()->addImage(testPath, LibImageDataService::instance()->getThumnailImageByPath(testPath));
    LibImageDataService::instance()->getMovieDurationStrByPath(testPath);
    LibImageDataService::instance()->getVisualIndex();
    LibImageDataService::instance()->getCount();
    LibImageDataService::instance()->setVisualIndex(0);
    LibImageDataService::instance()->addMovieDurationStr("", "");

    QTest::qWait(500);

    EXPECT_EQ(true, QFileInfo(QApplication::applicationDirPath() + "/jpg.jpg").isFile());
}

TEST_F(gtestview, LibBottomToolbar)
{
    LibBottomToolbar toolBar;
    toolBar.setAllFile(QApplication::applicationDirPath() + "/gif.gif", {QApplication::applicationDirPath() + "/gif.gif",
                                                                         QApplication::applicationDirPath() + "/tif.tif",
                                                                         QApplication::applicationDirPath() + "/jpg.jpg",
                                                                         QApplication::applicationDirPath() + "/jpg.jpg",
                                                                         QApplication::applicationDirPath() + "/svg.svg"
                                                                        });
    toolBar.getAllPath();

    QTestEventList e;
    e.addMousePress(Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10, 10), 100);
    e.addMouseMove(QPoint(100, 100), 50);
    e.addMouseRelease(Qt::MouseButton::LeftButton);
    e.addDelay(500);
    e.simulate(toolBar.m_imgListWidget);

    toolBar.setPictureDoBtnClicked(true);
    bool bRet = false;
    if (toolBar.getAllPath().count() > 0) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);
}

TEST_F(gtestview, UnionImage)
{
    //base utils
    Libutils::base::hash("12345");
    Libutils::base::mountDeviceExist("/media/uos/");
    Libutils::base::mountDeviceExist("/run/media/uos/");
    Libutils::base::onMountDevice("12345");
    Libutils::base::showInFileManager(QApplication::applicationDirPath() + "/jpg.jpg");
    Libutils::base::stringToDateTime("1970-01-01 00:00:00");
    Libutils::base::timeToString(QDateTime::fromString("1970-01-01 00:00:00"), true);
    Libutils::base::timeToString(QDateTime::fromString("1970-01-01 00:00:00"), false);

    //image utils
    Libutils::image::scaleImage(QApplication::applicationDirPath() + "/f827t3r9qwheo.icns");
    Libutils::image::scaleImage(QApplication::applicationDirPath() + "/jpg.jpg");
    Libutils::image::getCreateDateTime(QApplication::applicationDirPath() + "/jpg.jpg");
    Libutils::image::imageSupportSave(QApplication::applicationDirPath() + "/jpg.jpg");

    auto pix = Libutils::image::cachePixmap(QApplication::applicationDirPath() + "/jpg.jpg");
    Libutils::image::cutSquareImage(pix);
    Libutils::image::cutSquareImage(pix, {200, 200});

    Libutils::image::getImagesInfo(QApplication::applicationDirPath(), false);
    Libutils::image::getImagesInfo(QApplication::applicationDirPath(), true);
    Libutils::image::getOrientation(QApplication::applicationDirPath() + "/jpg.jpg");
    Libutils::image::getRotatedImage(QApplication::applicationDirPath() + "/jpg.jpg");

    Libutils::image::generateThumbnail(QApplication::applicationDirPath() + "/jpg.jpg");
    Libutils::image::generateThumbnail(QApplication::applicationDirPath() + "/errorPic.icns");
    Libutils::image::getThumbnail(QApplication::applicationDirPath() + "/jpg.jpg", true);
    Libutils::image::getThumbnail(QApplication::applicationDirPath() + "/jpg.jpg", false);

    Libutils::image::isCanRemove(QApplication::applicationDirPath() + "/jpg.jpg");
    Libutils::image::isCanRemove("ngw8uhrt8owjfpowsj");

    //Img Operate
    LibImgOperate worker;
    worker.slotMakeImgThumbnail(QApplication::applicationDirPath(), {QApplication::applicationDirPath() + "/jpg.jpg"}, 1, true);
    worker.slotMakeImgThumbnail(QApplication::applicationDirPath(), {QApplication::applicationDirPath() + "/jpg.jpg"}, 1, false);

    //snifferimageformat
    DetectImageFormat("nfoiehrf2oq3hjrfowhnefoi");
    DetectImageFormat(QApplication::applicationDirPath() + "/jpg.jpg");
    DetectImageFormat(QApplication::applicationDirPath() + "/gif.gif");
    DetectImageFormat(QApplication::applicationDirPath() + "/svg.svg");
    DetectImageFormat(QApplication::applicationDirPath() + "/mng.mng");
    DetectImageFormat(QApplication::applicationDirPath() + "/tif.tif");
    DetectImageFormat(QApplication::applicationDirPath() + "/wbmp.wbmp");
    DetectImageFormat(QApplication::applicationDirPath() + "/dds.dds");
    DetectImageFormat(QApplication::applicationDirPath() + "/ico.ico");
    DetectImageFormat(QApplication::applicationDirPath() + "/errorPic.icns");
    DetectImageFormat(QApplication::applicationDirPath() + "/tga.tga");

    //union image detial
    LibUnionImage_NameSpace::supportStaticFormat();
    LibUnionImage_NameSpace::supportMovieFormat();
    QImage res;
    LibUnionImage_NameSpace::creatNewImage(res);
    LibUnionImage_NameSpace::detectImageFormat(QApplication::applicationDirPath() + "/jpg.jpg");
    ASSERT_EQ(LibUnionImage_NameSpace::isNoneQImage(QImage()), true);
    LibUnionImage_NameSpace::rotateImage(90, res);
    LibUnionImage_NameSpace::rotateImage(80, res);
    res = QImage(QApplication::applicationDirPath() + "/jpg.jpg");
    LibUnionImage_NameSpace::rotateImage(90, res);
    QString err;
    LibUnionImage_NameSpace::rotateImageFIle(80, "", err);
    LibUnionImage_NameSpace::rotateImageFIle(90, QApplication::applicationDirPath() + "/svg.svg", err);
    LibUnionImage_NameSpace::rotateImageFIle(-90, QApplication::applicationDirPath() + "/svg.svg", err);
    LibUnionImage_NameSpace::rotateImageFIle(90, QApplication::applicationDirPath() + "/dds.dds", err);
    LibUnionImage_NameSpace::rotateImageFIle(-90, QApplication::applicationDirPath() + "/dds.dds", err);
    LibUnionImage_NameSpace::unionImageVersion();
    LibUnionImage_NameSpace::rotateImageFIleWithImage(-80, res, QApplication::applicationDirPath() + "/jpg.jpg", err);
    LibUnionImage_NameSpace::rotateImageFIleWithImage(-90, res, QApplication::applicationDirPath() + "/jpg.jpg", err);
    LibUnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath() + "/svg.svg", res, err);
    LibUnionImage_NameSpace::rotateImageFIleWithImage(-90, res, QApplication::applicationDirPath() + "/svg.svg", err);
    LibUnionImage_NameSpace::rotateImageFIleWithImage(90, res, QApplication::applicationDirPath() + "/svg.svg", err);
    LibUnionImage_NameSpace::loadStaticImageFromFile(QApplication::applicationDirPath() + "/tga.tga", res, err);
    LibUnionImage_NameSpace::rotateImageFIleWithImage(-90, res, QApplication::applicationDirPath() + "/tga.tga", err);
}
