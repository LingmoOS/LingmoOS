// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>
#include <gtest/gtest.h>

#define private public
#include "movieservice.h"

TEST(movieservice, movieInfo)
{
    QString filePath(QApplication::applicationDirPath() + "/500Kavi.avi");
    QFileInfo info(filePath);

    //总接口
    auto info_1 = MovieService::instance()->getMovieInfo(QUrl::fromLocalFile(filePath));

    //分接口
    auto info_2 = MovieService::instance()->getMovieInfo_mediainfo(info);
    auto info_3 = MovieService::instance()->getMovieInfo_ffmpeg(info);

    //简单判断
    ASSERT_EQ(info_1.fps, info_2.fps);

    if(MovieService::instance()->m_ffmpegExist) {
        ASSERT_EQ(info_2.fps, info_3.fps);
    } else {
        ASSERT_EQ(info_3.valid, false);
    }
}

TEST(movieservice, movieCover)
{
    QString filePath(QApplication::applicationDirPath() + "/500Kavi.avi");
    QFileInfo info(filePath);

    //总接口
    auto image_1 = MovieService::instance()->getMovieCover(QUrl::fromLocalFile(filePath), QApplication::applicationDirPath() + QDir::separator());

    //分接口
    auto image_2 = MovieService::instance()->getMovieCover_ffmpegthumbnailerlib(QUrl::fromLocalFile(filePath));
    auto image_3 = MovieService::instance()->getMovieCover_ffmpegthumbnailer(QUrl::fromLocalFile(filePath), QApplication::applicationDirPath() + QDir::separator());

    //简单判断
    ASSERT_EQ(image_1.isNull(), false);
    ASSERT_EQ(image_2.isNull(), false);
    if (MovieService::instance()->m_ffmpegExist) {
        ASSERT_FALSE(image_3.isNull());
    } else {
        ASSERT_TRUE(image_3.isNull());
    }
}
