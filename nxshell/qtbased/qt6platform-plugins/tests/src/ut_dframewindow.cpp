// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <qwindow.h>

#include "dframewindow.h"
#include "global.h"

DPP_USE_NAMESPACE

class TDFramewindow : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    QWindow *window = nullptr;
    DFrameWindow *frameWindow = nullptr;
};

void TDFramewindow::SetUp()
{
    window = new QWindow;
    frameWindow = new DFrameWindow(window);
}

void TDFramewindow::TearDown()
{
    delete window;
    delete frameWindow;
}

TEST_F(TDFramewindow, contentWindow)
{
    QWindow *w = frameWindow->contentWindow();
    ASSERT_TRUE(w);
    ASSERT_EQ(w, window);
}

TEST_F(TDFramewindow, shadowRadius)
{
    int shadowRadius = frameWindow->shadowRadius();
    ASSERT_EQ(shadowRadius, 60);
    frameWindow->setShadowRadius(80);
    shadowRadius = frameWindow->shadowRadius();
    ASSERT_EQ(shadowRadius, 80);
}

TEST_F(TDFramewindow, shadowOffset)
{
    QPoint shadowOffset = frameWindow->shadowOffset();
    ASSERT_EQ(shadowOffset, QPoint(0, 16));
    frameWindow->setShadowOffset(QPoint(0, 17));
    shadowOffset = frameWindow->shadowOffset();
    ASSERT_EQ(shadowOffset, QPoint(0, 17));
}

TEST_F(TDFramewindow, shadowColor)
{
    QColor shadowColor = frameWindow->shadowColor();
    ASSERT_EQ(shadowColor, QColor(0, 0, 0, 255 * 0.6));
    frameWindow->setShadowColor(QColor(0, 0, 0, 255 * 0.7));
    shadowColor = frameWindow->shadowColor();
    ASSERT_EQ(shadowColor, QColor(0, 0, 0, 255 * 0.7));
}

TEST_F(TDFramewindow, borderWidth)
{
    int borderWidth = frameWindow->borderWidth();
    ASSERT_EQ(borderWidth, 1);
    frameWindow->setBorderWidth(2);
    borderWidth = frameWindow->borderWidth();
    ASSERT_EQ(borderWidth, 2);
}

TEST_F(TDFramewindow, borderColor)
{
    QColor borderColor = frameWindow->borderColor();
    ASSERT_EQ(borderColor, QColor(0, 0, 0, 255 * 0.15));
    frameWindow->setBorderColor(QColor(0, 0, 0, 255 * 0.2));
    borderColor = frameWindow->borderColor();
    ASSERT_EQ(borderColor, QColor(0, 0, 0, 255 * 0.2));
}

TEST_F(TDFramewindow, setContentPath)
{
    QPainterPath path;
    path.addRect(0, 0, 100, 100);
    frameWindow->setContentPath(path);
    QPainterPath painterPath = frameWindow->contentPath();
    ASSERT_EQ(path, painterPath);
}

TEST_F(TDFramewindow, setContentRoundedRect)
{
    QRect rect{0, 0, 100, 100};
    frameWindow->setContentRoundedRect(rect, 8);
    QPainterPath painterPath = frameWindow->contentPath();
    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);
    ASSERT_EQ(path, painterPath);
}

TEST_F(TDFramewindow, contentMarginsHint)
{
    int radius = frameWindow->shadowRadius();
    QPoint offset = frameWindow->shadowOffset();
    int borderWidth = frameWindow->borderWidth();
    QMargins margins = QMargins(qMax(radius - offset.x(), borderWidth),
                       qMax(radius - offset.y(), borderWidth),
                       qMax(radius + offset.x(), borderWidth),
                       qMax(radius + offset.y(), borderWidth));
    QMargins actMargins = frameWindow->contentMarginsHint();
    ASSERT_EQ(margins, actMargins);
}

TEST_F(TDFramewindow, contentOffsetHint)
{
    QMargins margins = frameWindow->contentMarginsHint();
    QPoint point(margins.left(), margins.top());
    QPoint actPoint = frameWindow->contentOffsetHint();
    ASSERT_EQ(point, actPoint);
}

TEST_F(TDFramewindow, isClearContentAreaForShadowPixmap)
{
    bool isClear = frameWindow->isClearContentAreaForShadowPixmap(); 
    ASSERT_EQ(isClear, false); 
    frameWindow->setClearContentAreaForShadowPixmap(true);
    isClear = frameWindow->isClearContentAreaForShadowPixmap(); 
    ASSERT_EQ(isClear, true); 
}

TEST_F(TDFramewindow, isEnableSystemResize)
{
    bool enableSystemResize = frameWindow->isEnableSystemResize();
    ASSERT_EQ(enableSystemResize, true);
    frameWindow->setEnableSystemResize(false);
    enableSystemResize = frameWindow->isEnableSystemResize();
    ASSERT_EQ(enableSystemResize, false);
}

TEST_F(TDFramewindow, isEnableSystemMove)
{
    bool enableSystemMove = frameWindow->isEnableSystemMove();
    ASSERT_EQ(enableSystemMove, true);
    frameWindow->setEnableSystemMove(false);
    enableSystemMove = frameWindow->isEnableSystemMove();
    ASSERT_EQ(enableSystemMove, false);
}

TEST_F(TDFramewindow, redirectContent)
{
    bool redirectContent = frameWindow->redirectContent();
    ASSERT_EQ(redirectContent, false);
}

