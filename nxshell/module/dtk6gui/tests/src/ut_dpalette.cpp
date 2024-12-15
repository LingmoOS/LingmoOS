// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DGuiApplicationHelper>
#include <QDebug>
#include <QBuffer>

#include "test.h"
#include "dpalette.h"

DGUI_USE_NAMESPACE

class TDPalette : public DTestWithParam<int>
{
protected:
    void SetUp();
    void testAttribute(int cgCount, int ctCount);

    DPalette palette;
};

INSTANTIATE_TEST_CASE_P(DPalette, TDPalette, ::testing::Range(0, 12));

void TDPalette::SetUp()
{
    DGuiApplicationHelper::instance()->generatePalette(palette, DGuiApplicationHelper::LightType);
}

TEST_P(TDPalette, testFunction)
{
    // 测试属性设置和属性获取函数能够正常调用且返回正确数值
    enum {ColorGroupCount = 6};

    int ctGroup = GetParam();
    for (int i = 0; i < ColorGroupCount; ++i) {
        QColor color = palette.color(DPalette::ColorGroup(i), DPalette::ColorType(ctGroup));
        ASSERT_TRUE(color.isValid());

        QBrush brush = palette.brush(DPalette::ColorGroup(i), DPalette::ColorType(ctGroup));
        ASSERT_TRUE(brush.color().isValid());

        color = palette.color(DPalette::ColorType(ctGroup));
        ASSERT_TRUE(color.isValid());

        palette.setColor(DPalette::ColorGroup(i), DPalette::ColorType(ctGroup), Qt::black);
        color = palette.color(DPalette::ColorGroup(i), DPalette::ColorType(ctGroup));
        ASSERT_EQ(color, Qt::black);

        palette.setBrush(DPalette::ColorGroup(i), DPalette::ColorType(ctGroup), QBrush(Qt::blue));
        brush = palette.brush(DPalette::ColorGroup(i), DPalette::ColorType(ctGroup));
        ASSERT_EQ(brush.color(), Qt::blue);
    }

    palette.setColor(DPalette::ColorType(ctGroup), Qt::white);
    QColor color = palette.color(DPalette::ColorType(ctGroup));
    ASSERT_EQ(color, Qt::white);

    palette.setBrush(DPalette::ColorType(ctGroup), QBrush(Qt::blue));
    QBrush brush = palette.brush(DPalette::ColorType(ctGroup));
    ASSERT_EQ(brush.color(), Qt::blue);
}

TEST_F(TDPalette, testColorFunction)
{
    // 测试默认的调色板颜色有效
    QBrush brush = palette.itemBackground();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.textTitle();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.textTips();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.textWarning();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.textLively();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.lightLively();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.darkLively();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.frameBorder();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.placeholderText();
    ASSERT_TRUE(brush.color().isValid());

    brush = palette.frameShadowBorder();
    ASSERT_TRUE(brush.color().isValid());

#ifndef QT_NO_DATASTREAM
    QByteArray inArray;
    QDataStream in(&inArray, QIODevice::WriteOnly);
    // in.setVersion(QDataStream::Qt_5_11);

    // 直接调用左移运算符会出现二异性 先直接执行一次生成结果
    in << static_cast<const QPalette &>(palette);
    for (int i = 0; i < DPalette::NColorGroups; ++i) {
        for (int j = 0; j < DPalette::NColorTypes; ++j) {
            in << palette.brush(DPalette::ColorGroup(i), DPalette::ColorType(j));
        }
    }

    ASSERT_FALSE(inArray.isEmpty());

    DPalette tp;
    QDataStream out(&inArray, QIODevice::ReadOnly);
    out >> tp;

    ASSERT_EQ(palette, tp);
#endif

#ifndef QT_NO_DEBUG_STREAM
    QByteArray debugData;
    QBuffer debugBuffer(&debugData);
    ASSERT_TRUE(debugBuffer.open(QIODevice::WriteOnly));
    QDebug(&debugBuffer) << palette;
    debugBuffer.close();

    EXPECT_FALSE(debugData.isEmpty());
#endif
}
