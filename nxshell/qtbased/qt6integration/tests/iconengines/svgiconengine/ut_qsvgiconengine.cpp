/*
 * SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include <QPluginLoader>
#include <QIconEnginePlugin>
#include <QPainter>
#include <DSvgRenderer>
#include <QDebug>
#include <QIODevice>

#include "qsvgiconengine.h"

#define ICONNAME ":/icon_window_16px.svg"

DGUI_USE_NAMESPACE

class ut_QSvgIconEngine : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    QSvgIconEngine *m_IconEngine = nullptr;
    bool canUseRender = true;
};

void ut_QSvgIconEngine::SetUp()
{
    // 排除没有rsvg环境时测试的干扰
    DSvgRenderer render(QString(ICONNAME));
    if (!render.isValid())
        canUseRender = false;

    m_IconEngine = new QSvgIconEngine;
    QString fileName = ICONNAME;

    m_IconEngine->addFile(fileName, {}, QIcon::Normal, QIcon::Off);
}

void ut_QSvgIconEngine::TearDown()
{
    delete m_IconEngine;
}

TEST_F(ut_QSvgIconEngine, QSvgIconEngine)
{
    // 测试构造是否成功
    ASSERT_TRUE(m_IconEngine);
}

TEST_F(ut_QSvgIconEngine, paint)
{
    // 测试添加完原始数据后paint函数能够成功绘制数据
    QImage svgImage(QSize(16, 16), QImage::Format_ARGB32);
    QPainter imgPainter(&svgImage);

    m_IconEngine->paint(&imgPainter, QRect(QPoint(0, 0), QSize(16, 16)), QIcon::Normal, QIcon::Off);
    ASSERT_FALSE(svgImage.isNull());
}

TEST_F(ut_QSvgIconEngine, actualSize)
{
    // 测试icon的大小是否存在 没有rsvg环境时忽略
    QSize iconSize = m_IconEngine->actualSize(QSize(16, 16), QIcon::Normal, QIcon::Off);

    if (canUseRender) {
        ASSERT_FALSE(iconSize.isEmpty());
    } else {
        ASSERT_TRUE(iconSize.isEmpty());
    }
}

TEST_F(ut_QSvgIconEngine, pixmap)
{
    // 测试添加完数据后pixmap函数能够成功拿到数据
    QPixmap iconPixmap = m_IconEngine->pixmap(QSize(16, 16), QIcon::Normal, QIcon::Off);

    if (canUseRender) {
        ASSERT_FALSE(iconPixmap.isNull());
    } else {
        ASSERT_TRUE(iconPixmap.isNull());
    }
}

TEST_F(ut_QSvgIconEngine, key)
{
    ASSERT_FALSE(m_IconEngine->key().isEmpty());
}

TEST_F(ut_QSvgIconEngine, clone)
{
    auto clone = m_IconEngine->clone();
    ASSERT_TRUE(clone);
    delete clone;
}

TEST_F(ut_QSvgIconEngine, readAndWrite)
{
    // 测试读写函数是否正常，读写会返回正常的数值 进行比较
    QByteArray data;
    QDataStream inData(&data, QIODevice::WriteOnly);
    inData.setVersion(QDataStream::Qt_4_4);

    // 是否写入成功
    ASSERT_TRUE(m_IconEngine->write(inData));
    // 写入数据后 原始容器是否存在数据
    ASSERT_FALSE(data.isEmpty());

    QHash<int, QByteArray> svgBuffers;
    QHash<int, QString> svgFiles;
    int isCompressed = -1;

    QDataStream out(&data, QIODevice::ReadOnly);
    out.setVersion(QDataStream::Qt_4_4);

    out >> svgFiles >> isCompressed >> svgBuffers;

    if (canUseRender) {
        // 获取写出的数据是否存在
        ASSERT_FALSE(svgFiles.isEmpty());
        ASSERT_NE(isCompressed, -1);
        ASSERT_FALSE(svgBuffers.isEmpty());
    }

    svgFiles[-1] = QColor(Qt::red).name();
    data.clear();
    QDataStream inStream(&data, QIODevice::WriteOnly);
    inStream.setVersion(QDataStream::Qt_4_4);
    inStream << svgFiles << isCompressed << svgBuffers;

    QDataStream readSteam(&data, QIODevice::ReadOnly);
    readSteam.setVersion(QDataStream::Qt_4_4);
    // 测试读入数据是否成功
    ASSERT_TRUE(m_IconEngine->read(readSteam));
}

void testForAddFile(int *ret)
{
    *ret = -1;
#ifdef NDEBUG
    auto iconEngine = QSharedPointer<QSvgIconEngine>(new QSvgIconEngine);
    QString fileName = ICONNAME;

    // 死亡测试 优先进行 测试是否存在崩溃等问题
    ASSERT_DEBUG_DEATH(iconEngine->addFile(fileName, QSize(), QIcon::Normal, QIcon::Off), "");
#endif
    *ret = 0;
}

TEST(addFileDeathTest, addFile)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";
    int ret = -1;
    testForAddFile(&ret);
    // 如果函数崩溃 这里就不会等于0
    ASSERT_EQ(ret, 0);
}

struct TestPixmapType_Param {
    TestPixmapType_Param(const QPixmap &p, QIcon::Mode m, QIcon::State s)
        : pixmap(p)
        , mode(m)
        , state(s)
    {
    }

    QPixmap pixmap;
    QIcon::Mode mode;
    QIcon::State state;
};

class AddPixmapParamTest : public ::testing::TestWithParam<TestPixmapType_Param>
{
protected:
    void SetUp() override;
    void TearDown() override;

    QSvgIconEngine *engine;
};

INSTANTIATE_TEST_SUITE_P(QSvgIconEngine, AddPixmapParamTest, ::testing::Values(TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Disabled, QIcon::On), TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Active, QIcon::On), TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Normal, QIcon::On), TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Selected, QIcon::On), TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Disabled, QIcon::Off), TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Active, QIcon::Off), TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Normal, QIcon::Off), TestPixmapType_Param(QPixmap(ICONNAME), QIcon::Selected, QIcon::Off)));

TEST_P(AddPixmapParamTest, addPixmap)
{
    // 参数设置 批量进行不同参数  判断返回数据是否一致
    TestPixmapType_Param param = GetParam();
    engine->addPixmap(param.pixmap, param.mode, param.state);
    ASSERT_EQ(param.pixmap.toImage(), engine->pixmap({16, 16}, param.mode, param.state).toImage());
}

void AddPixmapParamTest::SetUp()
{
    engine = new QSvgIconEngine;
}

void AddPixmapParamTest::TearDown()
{
    delete engine;
}
