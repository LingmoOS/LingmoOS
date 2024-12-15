// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "gui/exhibition/preview/generalwidget/replicablelabel.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QEvent>
#include <QMouseEvent>
#include <QResizeEvent>

using namespace testing;
GRANDSEARCH_USE_NAMESPACE

TEST(ReplicableLabelTest, constructor)
{
    ReplicableLabel *label = new ReplicableLabel;

    ASSERT_TRUE(label);

    Qt::ContextMenuPolicy policy = label->contextMenuPolicy();
    EXPECT_EQ(policy, Qt::NoContextMenu);

    delete label;
}

TEST(ReplicableLabelTest, setText)
{
    ReplicableLabel label;

    QString test("test");
    label.setText(test);

    QString getVal = label.m_fullText;
    EXPECT_EQ(getVal, test);
}

TEST(ReplicableLabelTest, setElideMode)
{
    ReplicableLabel label;

    Qt::TextElideMode mode = Qt::ElideLeft;

    label.setElideMode(mode);

    Qt::TextElideMode getValue = label.m_elideMode;

    EXPECT_EQ(mode, getValue);
}

TEST(ReplicableLabelTest, elideMode)
{
    ReplicableLabel label;

    Qt::TextElideMode mode = Qt::ElideLeft;

    label.m_elideMode = mode;

    Qt::TextElideMode getValue = label.elideMode();

    EXPECT_EQ(mode, getValue);
}

TEST(ReplicableLabelTest, event)
{
    ReplicableLabel label;

    QEvent event(QEvent::FocusOut);

    label.event(&event);

    EXPECT_EQ(label.m_pressed, false);
}

TEST(ReplicableLabelTest, mousePressEvent)
{
    ReplicableLabel label;

    QMouseEvent event(QEvent::MouseButtonPress, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);

    label.mousePressEvent(&event);

    EXPECT_TRUE(label.m_pressed);
}

TEST(ReplicableLabelTest, mouseReleaseEvent)
{
    ReplicableLabel label;

    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);

    label.mouseReleaseEvent(&event);

    EXPECT_FALSE(label.m_pressed);
}

TEST(ReplicableLabelTest, mouseMoveEvent)
{
    ReplicableLabel label;

    label.m_pressed = true;

    QMouseEvent event(QEvent::MouseMove, QPointF(), QPointF(), QPointF(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier, Qt::MouseEventSynthesizedByQt);
    label.mouseMoveEvent(&event);

    EXPECT_FALSE(label.m_pressed);

    QString getVal = label.text();
    EXPECT_EQ(getVal, label.m_fullText);
}

TEST(ReplicableLabelTest, resizeEvent)
{
    ReplicableLabel label;

    QResizeEvent event(QSize(10, 10), QSize(20, 20));

    label.resizeEvent(&event);

    QString getVal = label.text();
    EXPECT_EQ(getVal, label.m_elideText);
}
