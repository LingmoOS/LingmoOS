/*
* Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     wuzhigang <wuzhigang@uniontech.com>
* Maintainer: wuzhigang <wuzhigang@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ImageItemTest.h"
#include "src/mainwindow.h"
#include "src/capplication.h"
#include "ac-deepin-camera-define.h"
#include "stub.h"
#include "stub_function.h"
#include "datamanager.h"
#include "stub/addr_pri.h"
#include "src/imageitem.h"
#include <QtTest/QTest>
#include <QShortcut>


ImageItemTest::ImageItemTest()
{

}

ImageItemTest::~ImageItemTest()
{

}

void ImageItemTest::SetUp()
{
    m_mainwindow = CamApp->getMainWindow();
    if (m_mainwindow) {
        m_imageItem = m_mainwindow->findChild<ImageItem *>(THUMBNAIL_PREVIEW);
    }
}

void ImageItemTest::TearDown()
{
    m_mainwindow = NULL;
}

QString ImageItemTest::getImageFileName()
{
    QString picPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                      + QDir::separator() + QObject::tr("Camera");
    QDir dir(picPath);
    QStringList filters;
    filters << QString("*.jpg");
    dir.setNameFilters(filters);
    QStringList Ilist = dir.entryList();
    if (!Ilist.empty()) {
        return  Ilist.first();
    }
    return "";
}

QString ImageItemTest::getVideoFileName()
{
    QString videoPath = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
                        + QDir::separator() + QObject::tr("Camera");
    QDir dir(videoPath);
    QStringList filters;
    filters << QString("*.webm");
    dir.setNameFilters(filters);
    QStringList Ilist = dir.entryList();
    if (!Ilist.empty()) {
        return  Ilist.first();
    }
    return "";
}


/**
 *  @brief 打印窗口
 */
TEST_F(ImageItemTest, printdialog)
{
    QAction *print = m_mainwindow->findChild<QAction *>("PrinterAction");
    if (print)
        print->trigger();
    QTest::qWait(1000);
}


/**
 *  @brief 获取焦点
 */
TEST_F(ImageItemTest, Focus)
{
    m_imageItem->setFocus();
    QTest::qWait(100);
}


/**
 *  @brief 打开右键菜单
 */
TEST_F(ImageItemTest, RightMenu)
{
    emit m_imageItem->customContextMenuRequested(QPoint(0, 0));
    QTest::qWait(100);
    QTest::mouseClick(m_mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 拷贝
 */
TEST_F(ImageItemTest, Copy)
{
    QTest::keySequence(m_imageItem, QKeySequence("ctrl+c"));
    QTest::qWait(100);
    QTest::mouseClick(m_mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 删除
 */
TEST_F(ImageItemTest, deleteFile)
{
    QTest::keySequence(m_imageItem, QKeySequence("delete"));
    QTest::qWait(100);
    QTest::mouseClick(m_mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 打开文件夹
 */
TEST_F(ImageItemTest, openFolder)
{
    QTest::keySequence(m_imageItem, QKeySequence("Ctrl+O"));
    QTest::qWait(100);
    QTest::mouseClick(m_mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 打印
 */
TEST_F(ImageItemTest, print)
{
    QTest::keySequence(m_imageItem, QKeySequence("Ctrl+P"));
    QTest::qWait(100);
    QTest::mouseClick(m_mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 菜单
 */
TEST_F(ImageItemTest, KeyAltAddM)
{
    QTest::keySequence(m_imageItem, QKeySequence("Alt+M"));
    QTest::qWait(100);
    QTest::mouseClick(m_mainwindow, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 100);
}

/**
 *  @brief 右键打开文件夹
 */
TEST_F(ImageItemTest, OpenFolder)
{
    QAction *actOpenFolder = m_mainwindow->findChild<QAction *>("OpenFolderAction");
    if (actOpenFolder) {
        actOpenFolder->trigger();
    }
    //TODO  找到打开的文管窗口，然后关闭
}

/**
 *  @brief 单击缩略图
 */
TEST_F(ImageItemTest, mouseClickEvent)
{
    if (m_imageItem) {
        QTest::mouseClick(m_imageItem, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    }
    QTest::qWait(1000);
    //TODO 找到打开的看图或者影院窗口，关闭
}

/**
 *  @brief 删除单张缩略图
 */
TEST_F(ImageItemTest, event)
{
    if (m_imageItem) {
        QTest::qWait(1000);
        QTest::mouseMove(m_imageItem, QPoint(0, 0), 500);
        QTest::mousePress(m_imageItem, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
        QTest::mouseRelease(m_imageItem, Qt::LeftButton, Qt::NoModifier, QPoint(0, 0), 500);
    }
}

/**
 *  @brief 缩略图复制，删除
 */
TEST_F(ImageItemTest, ImageItemCopyDel)
{
    QAction *copyact = m_imageItem->findChild<QAction *>("CopyAction");
    if (copyact)
        copyact->trigger();

    QAction *delact = m_imageItem->findChild<QAction *>("DelAction");
    if (delact)
        delact->trigger();
}

/**
 *  @brief 更换为图片
 */
TEST_F(ImageItemTest, ChangeToPic)
{
    QString ImageFile = getImageFileName();
    qInfo() << "get image:" << ImageFile;
    if (ImageFile.isEmpty()) {
        return;
    }
    m_imageItem->updatePicPath(ImageFile);
    emit m_imageItem->customContextMenuRequested(QPoint(0, 0));
    QTest::qWait(100);
    m_imageItem->openFile();
    m_imageItem->onPrint();
    m_imageItem->onShowMenu();
}

/**
 *  @brief 更换为视频
 */
TEST_F(ImageItemTest, ChangeToVideo)
{
    QString videoFile = getVideoFileName();
    qInfo() << "get video:" << videoFile;
    if (videoFile.isEmpty()) {
        return;
    }
//    m_imageItem->updatePicPath(videoFile);
    emit m_imageItem->customContextMenuRequested(QPoint(0, 0));
    QTest::qWait(100);
    m_imageItem->openFile();
    m_imageItem->onPrint();
    m_imageItem->onShowMenu();
}
