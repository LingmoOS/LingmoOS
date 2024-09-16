// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtestview.h"
#include "accessibility/ac-desktop-define.h"
#include <DGuiApplicationHelper>
#include "imageviewer.h"

TEST_F(gtestview, imageviewer_startSlideShow)
{
    QStringList list;
    list << QApplication::applicationDirPath() + "/gif.gif";
    list << QApplication::applicationDirPath() + "/gif2.gif";
    list << QApplication::applicationDirPath() + "/ico.ico";
    list << QApplication::applicationDirPath() + "/jpg.jpg";
    list << QApplication::applicationDirPath() + "/png.png";
    list << QApplication::applicationDirPath() + "/mng.mng";
    list << QApplication::applicationDirPath() + "/svg1.svg";
    list << QApplication::applicationDirPath() + "/svg2.svg";
    list << QApplication::applicationDirPath() + "/svg3.svg";
    QString path = QApplication::applicationDirPath() + "/gif.gif";

    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr);

    m_imageViewer->startSlideShow(list, path);

    QTest::qWait(5000);
    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;

    EXPECT_EQ(true, QFileInfo(path).isFile());
}

TEST_F(gtestview, imageviewer_getCurrentPath)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeLocal, CACHE_PATH, nullptr);

    QString getPath = m_imageViewer->getCurrentPath();

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;

    EXPECT_EQ(false, QFileInfo(getPath).isFile());
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeNext)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeNext);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeBack)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeBack);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypePre)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypePre);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeAdaptImage)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeAdaptImage);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeAdaptScreen)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeAdaptScreen);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeCollection)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeCollection);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeOcr)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeOcr);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeRotateLeft)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeRotateLeft);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeRotateRight)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeRotateRight);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_getBottomtoolbarButton_ButtonTypeTrash)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    DIconButton *iconButton = m_imageViewer->getBottomtoolbarButton(imageViewerSpace::ButtonTypeTrash);

    bool bRet = false;
    if (iconButton) {
        iconButton->click();
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_setViewPanelContextMenuItemVisible)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    m_imageViewer->setViewPanelContextMenuItemVisible(imageViewerSpace::IdFullScreen, false);

    bool bRet = false;
    if (m_imageViewer) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_setBottomToolBarButtonAlawysNotVisible)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    m_imageViewer->setBottomToolBarButtonAlawysNotVisible(imageViewerSpace::ButtonTypeNext, false);
    m_imageViewer->setBottomToolBarButtonAlawysNotVisible(imageViewerSpace::ButtonTypeNext, true);

    bool bRet = false;
    if (m_imageViewer) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_setCustomAlbumName)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    QMap<QString, bool> map;
    map.insert("xxx", true);
    bool isFav = true;
    m_imageViewer->setCustomAlbumName(map, isFav);

    bool bRet = false;
    if (m_imageViewer) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_setCustomAlbumNameAndUID)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    QMap<int, std::pair<QString, bool>> map;
    map.insert(0, std::make_pair("xxx", true));
    bool isFav = true;
    m_imageViewer->setCustomAlbumNameAndUID(map, isFav);

    bool bRet = false;
    if (m_imageViewer) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

TEST_F(gtestview, imageviewer_setDropEnabled)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    m_imageViewer->setDropEnabled(true);

    bool bRet = false;
    if (m_imageViewer) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}

//switchFullScreen

TEST_F(gtestview, imageviewer_switchFullScreen)
{
    QString CACHE_PATH = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                         + QDir::separator() + "deepin" + QDir::separator() + "image-view-plugin";

    ImageViewer *m_imageViewer = new ImageViewer(imageViewerSpace::ImgViewerType::ImgViewerTypeAlbum, CACHE_PATH, nullptr);

    m_imageViewer->switchFullScreen();

    bool bRet = false;
    if (m_imageViewer) {
        bRet = true;
    }
    EXPECT_EQ(true, bRet);

    m_imageViewer->deleteLater();
    m_imageViewer = nullptr;
}
