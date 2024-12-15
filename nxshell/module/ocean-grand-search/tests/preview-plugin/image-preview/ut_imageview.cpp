// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "image-preview/imagepreview_global.h"
#include "image-preview/imageview.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <DLabel>

#include <QSize>
#include <QMovie>
#include <QFileInfo>

DWIDGET_USE_NAMESPACE;
IMAGE_PREVIEW_USE_NAMESPACE

using namespace testing;

TEST(ImageViewTest, constructor)
{
    ImageView *view = new ImageView;

    EXPECT_TRUE(view);
    EXPECT_TRUE(view->m_imageLabel);
    EXPECT_TRUE(view->m_titleLabel);

    delete view;
}

TEST(ImageViewTest, sourceSize)
{
    ImageView view;

    QSize size(100, 100);

    view.m_sourceSize = size;

    QSize result = view.sourceSize();
    EXPECT_EQ(result, size);
}

TEST(ImageViewTest, stopPreview)
{
    ImageView view;
    ASSERT_FALSE(view.m_movie);

    QMovie *movie = new QMovie;
    view.m_movie = movie;

    bool result = view.stopPreview();
    EXPECT_TRUE(result);

    view.m_movie = nullptr;
    delete movie;
    movie = nullptr;
}

TEST(ImageViewTest, loadImage)
{
    ImageView view;

    QString file("testFile.gif");
    QString type("gif");

    stub_ext::StubExt stu;

    QString ut_name("ABCDEFGHIJKLMNOPQRSTUVWXYZZYXWVUTSRQPONMLKJIHGFEDCBA"
                    "abcdefghijklmnopqrstuvwxyzzyxwvutsrqponmlkjihgfedcba");

    stu.set_lamda(&QFileInfo::fileName, [&]() {
        return ut_name;
    });

    // 显示错误页面
    bool ut_isReadable = false;
    stu.set_lamda(&QFileInfo::isReadable, [&](){
        return ut_isReadable;
    });

    bool ut_call_showErrorPage = false;
    stu.set_lamda(&ImageView::showErrorPage, [&]() {
        ut_call_showErrorPage = true;
    });

    view.m_titleLabel->setFixedWidth(100);

    view.loadImage(file, type);
    EXPECT_TRUE(ut_call_showErrorPage);

    // 正常显示
    ut_isReadable = true;
    bool ut_canPreview = true;
    stu.set_lamda(&ImageView::canPreview, [&]() {
        return ut_canPreview;
    });

    // gif图片
    QRect ut_rect(0, 0, 50, 50);
    stu.set_lamda(&QMovie::frameRect, [&]() {
        return ut_rect;
    });

    view.loadImage(file, type);
    EXPECT_TRUE(view.m_movie);

    // 其余格式图片
    file = "testFile.png";
    type = "png";

    // 加载失败
    bool ut_load = false;
    stu.set_lamda((bool(QImage::*)(const QString &, const char *format))ADDR(QImage, load), [&]() {
        return ut_load;
    });

    ut_call_showErrorPage = false;
    view.loadImage(file, type);
    EXPECT_TRUE(ut_call_showErrorPage);

    // 加载成功
    ut_load = true;

    stu.set_lamda(&QImage::size, [&]() {
        return ut_rect.size();
    });

    bool ut_call_setPixmap = false;
    stu.set_lamda(&QLabel::setPixmap, [&]() {
       ut_call_setPixmap = true;
    });

    view.loadImage(file, type);
    EXPECT_TRUE(ut_call_setPixmap);
}

TEST(ImageViewTest, canPreview)
{
    ImageView view;

    bool result = view.canPreview();
    EXPECT_FALSE(result);
}

TEST(ImageViewTest, getRoundPixmap)
{
    ImageView view;

    QPixmap result = view.getRoundPixmap(QPixmap());

    EXPECT_EQ(result.width(), 310);
    EXPECT_EQ(result.height(), 110);
}

TEST(ImageViewTest, onMovieFrameChanged)
{
    ImageView view;

    stub_ext::StubExt stu;

    bool ut_call_getRoundPixmap = false;
    stu.set_lamda(&ImageView::getRoundPixmap, [&]() {
        ut_call_getRoundPixmap = true;
        return QPixmap();
    });

    QMovie *movie = new QMovie;
    view.m_movie = movie;

    view.onMovieFrameChanged(1);
    EXPECT_TRUE(ut_call_getRoundPixmap);

    view.m_movie = nullptr;
    delete movie;
    movie = nullptr;
}

TEST(ImageViewTest, showErrorPage)
{
    ImageView view;

    stub_ext::StubExt stu;

    bool ut_call_getRoundPixmap = false;
    stu.set_lamda(&ImageView::getRoundPixmap, [&]() {
        ut_call_getRoundPixmap = true;
        return QPixmap();
    });

    view.showErrorPage();
    EXPECT_TRUE(ut_call_getRoundPixmap);
}


