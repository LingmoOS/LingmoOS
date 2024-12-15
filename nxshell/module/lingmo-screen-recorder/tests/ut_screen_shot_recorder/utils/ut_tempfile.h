// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QScreen>
#include <QPixmap>
#include <QApplication>
#include <QDesktopWidget>
#include <gtest/gtest.h>
#include "../../src/utils/tempfile.h"


using namespace testing;

class TempFileTest:public testing::Test{

public:
    TempFile* tempFile = nullptr;
    QScreen *m_primaryScreen;
    QPixmap m_pix;
    virtual void SetUp() override{
        tempFile = TempFile::instance();
        m_primaryScreen = QGuiApplication::primaryScreen();
        m_pix = m_primaryScreen->grabWindow(QApplication::desktop()->winId());
        std::cout << "start TempFileTest" << std::endl;
    }

    virtual void TearDown() override{
        //delete tempFile;
        std::cout << "end TempFileTest" << std::endl;
    }
};


TEST_F(TempFileTest, setFullScreenPixmap)
{
    tempFile->setFullScreenPixmap(m_pix);
    tempFile->getFullscreenPixmap();
    EXPECT_NE(nullptr, tempFile);
}

TEST_F(TempFileTest, setBlurPixmap)
{
    tempFile->setBlurPixmap(m_pix);
    tempFile->getBlurPixmap();
    EXPECT_NE(nullptr, tempFile);
}

TEST_F(TempFileTest, setMosaicPixmap)
{
    tempFile->setMosaicPixmap(m_pix);
    tempFile->getMosaicPixmap();
    EXPECT_NE(nullptr, tempFile);
}
