/*
 * SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include "qsvgiohandler.h"

#include <gtest/gtest.h>

#include <DSvgRenderer>
#include <QColor>
#include <QFile>
#include <QImage>
#include <QRect>
#include <QVariant>
#include <QDebug>

#ifndef UT_QSVGIOHANDLERPRIVATE
#define UT_QSVGIOHANDLERPRIVATE

DGUI_USE_NAMESPACE
class QSvgIOHandlerPrivate
{
public:
    QSvgIOHandlerPrivate(QSvgIOHandler *qq);
    bool load(QIODevice *device);

    QSvgIOHandler *q;
    DSvgRenderer r;
    QSize defaultSize;
    QRect clipRect;
    QSize scaledSize;
    QRect scaledClipRect;
    bool loaded;
    bool readDone;
    QColor backColor;
};
#endif

class ut_QSvgIOHandler : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    QSvgIOHandler *handler;
};

void ut_QSvgIOHandler::SetUp()
{
    handler = new QSvgIOHandler();
}

void ut_QSvgIOHandler::TearDown()
{
    delete handler;
}

TEST_F(ut_QSvgIOHandler, testForPrivatePtr)
{
    ASSERT_TRUE(handler->d);
    ASSERT_FALSE(handler->d->loaded);
    ASSERT_FALSE(handler->d->readDone);
    ASSERT_EQ(handler->d->backColor, Qt::transparent);
}

static const char *testTmpFile = "/tmp/tmpSvg.svg";
static const char *testSvgContent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                    "<svg width=\"32px\" height=\"32px\" viewBox=\"0 0 32 32\" version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
                                    "<title>logo icon</title>"
                                    "<defs>"
                                    "<linearGradient x1=\"50%\" y1=\"0%\" x2=\"50%\" y2=\"100%\" id=\"linearGradient-1\">"
                                    "<stop stop-color=\"#00F1FF\" offset=\"0%\"></stop>"
                                    "<stop stop-color=\"#008AFF\" offset=\"100%\"></stop>"
                                    "</linearGradient>"
                                    "</defs>"
                                    "<g id=\"light\" stroke=\"none\" stroke-width=\"1\" fill=\"none\" fill-rule=\"evenodd\">"
                                    "<g id=\"1\" transform=\"translate(-460.000000, -159.000000)\">"
                                    "<g id=\"tittlebar\" transform=\"translate(450.000000, 150.000000)\">"
                                    "<g id=\"logo-icon\" transform=\"translate(10.000000, 9.000000)\">"
                                    "<g>"
                                    "<rect id=\"矩形\" fill=\"url(#linearGradient-1)\" x=\"0\" y=\"0\" width=\"32\" height=\"32\" rx=\"8\"></rect>"
                                    "<path d=\"M23,25 L23,24.0002278 C21.6666667,24.0002278 21,24.0002278 21,24.0002278 L21,20 L22,20 L22,19 L21,19 L21,17 L20,17.5 L20,19 L19,19 L19,20 L20,20 L20,24.0002278 C20,24.0002278 20,24.0002278 20,24.0002278 C20,24.5523868 20.4476132,25 20.9997722,25 C20.9998481,25 20.9999241,25 21,25 L23,25 Z M24.0210414,23.9789586 L24.0210414,22.2215556 L24.0210414,22.2215556 L24.6565969,21.6226667 L27.2207674,25 L29.0052118,25 L25.6099302,20.7304444 L28.4678057,18.0074755 L26.6711391,18.0074755 L24.0210414,20.6204444 L24,15.5 L23,15 L23,25 C23.5639056,25 24.0210414,24.5428642 24.0210414,23.9789586 Z\" id=\"tk\" fill=\"#FFFFFF\" fill-rule=\"nonzero\"></path>"
                                    "<path d=\"M10.275,24 C13.175,24 15.35,23.2 16.85,21.6 C18.275,20.075 19,17.825 19,15 C19,12.15 18.275,10.05 16.85,8.55 C15.35,6.95 13.175,6 10.275,6 L4,6 L4,24 L10.275,24 Z M9.725,21 L7,21 L7,9 L9.725,9 C11.95,9 13.575,9.15 14.6,10.175 C15.6,11.175 16,12.75 16,15 C16,17.2 15.6,18.9 14.6,19.95 C13.575,20.975 11.95,21 9.725,21 Z\" id=\"D\" fill=\"#FFFFFF\" fill-rule=\"nonzero\"></path>"
                                    "</g>"
                                    "</g>"
                                    "</g>"
                                    "</g>"
                                    "</g>"
                                    "</svg>";

TEST_F(ut_QSvgIOHandler, testForLoad)
{
    QImage image(QSize(20, 20), QImage::Format_RGB32);
    // 填充一个底色用于判断颜色是否被覆盖
    image.fill(Qt::green);

    ASSERT_FALSE(handler->read(&image));

    QFile tmpFile(testTmpFile);

    ASSERT_TRUE(tmpFile.open(QFile::WriteOnly));
    tmpFile.write(testSvgContent);
    tmpFile.close();
    ASSERT_TRUE(tmpFile.open(QFile::ReadWrite));
    ASSERT_TRUE(handler->canRead(&tmpFile));

    QRect clipRect(0, 0, 10, 10);
    handler->setOption(QSvgIOHandler::ClipRect, clipRect);
    ASSERT_EQ(handler->option(QSvgIOHandler::ClipRect), clipRect);

    QSize scaleSize(20, 20);
    handler->setOption(QSvgIOHandler::ScaledSize, scaleSize);
    ASSERT_EQ(handler->option(QSvgIOHandler::ScaledSize), scaleSize);

    QRect scaledClipRect(2, 2, 5, 5);
    handler->setOption(QSvgIOHandler::ScaledClipRect, scaledClipRect);
    ASSERT_EQ(handler->option(QSvgIOHandler::ScaledClipRect), scaledClipRect);

    QColor backgroundColor(Qt::blue);
    handler->setOption(QSvgIOHandler::BackgroundColor, backgroundColor);
    ASSERT_EQ(handler->option(QSvgIOHandler::BackgroundColor), backgroundColor);
    ASSERT_TRUE(handler->option(QSvgIOHandler::Size).isValid());

    handler->setDevice(&tmpFile);
    handler->read(&image);
    tmpFile.close();
    tmpFile.remove();
}

TEST_F(ut_QSvgIOHandler, testFunction)
{
    ASSERT_STREQ(handler->name(), "svg");

    ASSERT_TRUE(handler->supportsOption(QSvgIOHandler::ImageFormat));
    ASSERT_TRUE(handler->supportsOption(QSvgIOHandler::Size));
    ASSERT_TRUE(handler->supportsOption(QSvgIOHandler::ClipRect));
    ASSERT_TRUE(handler->supportsOption(QSvgIOHandler::ScaledSize));
    ASSERT_TRUE(handler->supportsOption(QSvgIOHandler::ScaledClipRect));
    ASSERT_TRUE(handler->supportsOption(QSvgIOHandler::BackgroundColor));
    ASSERT_FALSE(handler->supportsOption(QSvgIOHandler::ImageTransformation));
}
