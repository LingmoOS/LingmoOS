// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Utils.h"
#include "Global.h"
#include "stub.h"
#include "DocSheet.h"

#include <QTest>
#include <QTimer>
#include <QClipboard>
#include <QProcessEnvironment>

#include <gtest/gtest.h>

/********测试Utils***********/
class TestUtils : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();
};
void TestUtils::SetUp()
{

}

void TestUtils::TearDown()
{

}
/*************桩函数****************/
static QString g_funcName;
void setText_stub(const QString &, QClipboard::Mode)
{
    g_funcName = __FUNCTION__;
}

QString value_stub1(const QString &, const QString &)
{
    return "wayland";
}


QString value_stub2(const QString &, const QString &)
{
    return "x11";
}
/*************测试用例****************/
TEST_F(TestUtils, UT_Utils_getKeyshortcut_001)
{
    QEvent::Type type = QEvent::KeyPress;
    int key = Qt::Key_0;
    Qt::KeyboardModifiers modifiers = Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::ShiftModifier;
    QKeyEvent keyEvent(type, key, modifiers);
    EXPECT_TRUE(Utils::getKeyshortcut(&keyEvent) == "Ctrl+Alt+Meta+Shift+0");
}

TEST_F(TestUtils, UT_Utils_getInputDataSize_001)
{
    EXPECT_TRUE(Utils::getInputDataSize(1023) == "1023 B");
    EXPECT_TRUE(Utils::getInputDataSize(1024 * 1023) == "1023.0 KB");
    EXPECT_TRUE(Utils::getInputDataSize(1024 * 1024 * 1023) == "1023.0 MB");
    EXPECT_TRUE(Utils::getInputDataSize(1024 * 1024 * 1025) == "1.0 GB");
}

TEST_F(TestUtils, UT_Utils_roundQPixmap_001)
{
    QPixmap img_in;
    int radius = 0;

    EXPECT_TRUE(Utils::roundQPixmap(img_in, radius).isNull());

    img_in = QPixmap(20, 20);
    EXPECT_FALSE(Utils::roundQPixmap(img_in, radius).isNull());

}

TEST_F(TestUtils, UT_Utils_copyText_001)
{
    Stub s;
    s.set(ADDR(QClipboard, setText), setText_stub);

    QString sText("test");
    Utils::copyText(sText);

    EXPECT_TRUE(g_funcName == "setText_stub");
}

TEST_F(TestUtils, UT_Utils_getElidedText_001)
{
    QFont font;
    QFontMetrics fontMetrics(font);
    QSize size(100, 40);
    QString text("This is a test document. This is a test document. This is a test document.");
    Qt::Alignment alignment = Qt::AlignTop | Qt::AlignLeft;

    EXPECT_TRUE(Utils::getElidedText(fontMetrics, size, text, alignment).endsWith("..."));
}

TEST_F(TestUtils, UT_Utils_copyFile_001)
{
    QString sourcePath = UTSOURCEDIR;
    sourcePath += "/files/normal.txt";

    QString destinationPath = UTSOURCEDIR;
    destinationPath += "/files/copy.txt";

    EXPECT_TRUE(Utils::copyFile(sourcePath, sourcePath));

    EXPECT_TRUE(Utils::copyFile(sourcePath, destinationPath));
}

TEST_F(TestUtils, UT_Utils_copyImage_001)
{
    QString sourcePath = UTSOURCEDIR;
    sourcePath += "/files/normal.png";
    QImage srcimg(sourcePath);

    EXPECT_TRUE(Utils::copyImage(srcimg, 0, 0, 0, 0).isNull());

    EXPECT_FALSE(Utils::copyImage(srcimg, 0, 0, 100, 100).isNull());

    QImage srcimg2(QSize(100, 100), QImage::Format_Mono);
    EXPECT_FALSE(Utils::copyImage(srcimg2, 0, 0, 100, 100).isNull());

    QImage srcimg3(QSize(100, 100), QImage::Format_MonoLSB);
    EXPECT_FALSE(Utils::copyImage(srcimg3, 0, 0, 100, 100).isNull());
}

TEST_F(TestUtils, UT_Utils_getHiglightColorList_001)
{
    EXPECT_TRUE(Utils::getHiglightColorList().size() == 8);
}

TEST_F(TestUtils, UT_Utils_setHiglightColorIndex_001)
{
    Utils::setHiglightColorIndex(2);
    EXPECT_TRUE(Utils::m_colorIndex == 2);
}

TEST_F(TestUtils, UT_Utils_getCurHiglightColor_001)
{
    EXPECT_TRUE(Utils::getCurHiglightColor() == QColor("#9023FC"));
}

TEST_F(TestUtils, UT_Utils_setObjectNoFocusPolicy_001)
{
    QWidget w;
    QWidget w1(&w);
    Utils::setObjectNoFocusPolicy(&w);
}

TEST_F(TestUtils, UT_Utils_isWayland_001)
{
    Stub s;
    s.set(ADDR(QProcessEnvironment, value), value_stub1);

    EXPECT_TRUE(Utils::isWayland());
}

TEST_F(TestUtils, UT_Utils_isWayland_002)
{
    Stub s;
    s.set(ADDR(QProcessEnvironment, value), value_stub2);

    EXPECT_FALSE(Utils::isWayland());
}
