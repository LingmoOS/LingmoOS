// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QString>
#include <QTest>
#include <QMainWindow>
#include <QHBoxLayout>
#include  <QFont>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/waylandrecord/waylandintegration.h"
#include "../../src/waylandrecord/waylandintegration_p.h"


using namespace testing;


using namespace WaylandIntegration;

class WaylandOutputTest: public testing::Test
{
public:
    WaylandOutput m_waylandOutput;
    virtual void SetUp() override
    {
        std::cout << "start UtilsTest" << std::endl;

    }

    virtual void TearDown() override
    {
        std::cout << "end UtilsTest" << std::endl;
    }
};

ACCESS_PRIVATE_FIELD(WaylandOutput, QString, m_manufacturer);
TEST_F(WaylandOutputTest, setManufacturer)
{
    m_waylandOutput.setManufacturer("test");
    QString res =  access_private_field::WaylandOutputm_manufacturer(m_waylandOutput);
    EXPECT_EQ("test", res);
}

TEST_F(WaylandOutputTest, manufacturer)
{
    m_waylandOutput.setManufacturer("test");
    EXPECT_EQ("test", m_waylandOutput.manufacturer());
}

ACCESS_PRIVATE_FIELD(WaylandOutput, QString, m_model);
TEST_F(WaylandOutputTest, setModel)
{
    m_waylandOutput.setModel("test");
    QString res =  access_private_field::WaylandOutputm_model(m_waylandOutput);
    EXPECT_EQ("test", res);
}

TEST_F(WaylandOutputTest, model)
{
    m_waylandOutput.setModel("test");
    EXPECT_EQ("test", m_waylandOutput.model());
}

ACCESS_PRIVATE_FIELD(WaylandOutput, QSize, m_resolution);
TEST_F(WaylandOutputTest, setResolution)
{
    m_waylandOutput.setResolution(QSize(100, 100));
    QSize res =  access_private_field::WaylandOutputm_resolution(m_waylandOutput);
    EXPECT_EQ(QSize(100, 100), res);
}

TEST_F(WaylandOutputTest, resolution)
{
    m_waylandOutput.setResolution(QSize(100, 100));
    EXPECT_EQ(QSize(100, 100), m_waylandOutput.resolution());
}

ACCESS_PRIVATE_FIELD(WaylandOutput, WaylandOutput::OutputType, m_outputType);
TEST_F(WaylandOutputTest, setOutputType)
{
    m_waylandOutput.setOutputType("IDP");
    WaylandOutput::OutputType res =  access_private_field::WaylandOutputm_outputType(m_waylandOutput);
    EXPECT_EQ(WaylandOutput::OutputType::Laptop, res);
}

TEST_F(WaylandOutputTest, outputType)
{
    m_waylandOutput.setOutputType("IDP");
    EXPECT_EQ(WaylandOutput::OutputType::Laptop, m_waylandOutput.outputType());
    m_waylandOutput.setOutputType("VGA");
    EXPECT_EQ(WaylandOutput::OutputType::Monitor, m_waylandOutput.outputType());
    m_waylandOutput.setOutputType("TV");
    EXPECT_EQ(WaylandOutput::OutputType::Television, m_waylandOutput.outputType());
    m_waylandOutput.setOutputType("test");
    EXPECT_EQ(WaylandOutput::OutputType::Monitor, m_waylandOutput.outputType());
}

ACCESS_PRIVATE_FIELD(WaylandOutput, int, m_waylandOutputName);
TEST_F(WaylandOutputTest, setWaylandOutputName)
{
    m_waylandOutput.setWaylandOutputName(1);
    int res =  access_private_field::WaylandOutputm_waylandOutputName(m_waylandOutput);
    EXPECT_EQ(1, res);
}

TEST_F(WaylandOutputTest, waylandOutputName)
{
    m_waylandOutput.setWaylandOutputName(1);
    EXPECT_EQ(1, m_waylandOutput.waylandOutputName());
}

ACCESS_PRIVATE_FIELD(WaylandOutput, int, m_waylandOutputVersion);
TEST_F(WaylandOutputTest, setWaylandOutputVersion)
{
    m_waylandOutput.setWaylandOutputVersion(1);
    int res =  access_private_field::WaylandOutputm_waylandOutputVersion(m_waylandOutput);
    EXPECT_EQ(1, res);
}

TEST_F(WaylandOutputTest, waylandOutputVersion)
{
    m_waylandOutput.setWaylandOutputVersion(1);
    EXPECT_EQ(1, m_waylandOutput.waylandOutputVersion());
}

class WaylandIntegrationTest: public testing::Test
{
public:
    Stub stub;
    virtual void SetUp() override
    {
        std::cout << "start UtilsTest" << std::endl;

    }

    virtual void TearDown() override
    {
        std::cout << "end UtilsTest" << std::endl;
    }
};

void initWayland_stub(QStringList list)
{

}

TEST_F(WaylandIntegrationTest, init)
{
    QStringList arguments;
    arguments << QString("%1").arg(3);
    arguments << QString("%1").arg(1000) << QString("%1").arg(500);
    arguments << QString("%1").arg(0) << QString("%1").arg(0);
    arguments << QString("%1").arg(24);
    arguments << QString("%1").arg("tmp/test.mkv");
    arguments << QString("%1").arg(4);
    qDebug() << arguments;

    stub.set(ADDR(WaylandIntegrationPrivate, initWayland), initWayland_stub);
    init(arguments);
    stub.reset(ADDR(WaylandIntegrationPrivate, initWayland));
}

bool isEGLInitialized_stub()
{
    return true;
}

TEST_F(WaylandIntegrationTest, isEGLInitialized)
{
    stub.set(ADDR(WaylandIntegrationPrivate, isEGLInitialized), isEGLInitialized_stub);
    bool flag = isEGLInitialized();
    EXPECT_EQ(true, flag);
    stub.reset(ADDR(WaylandIntegrationPrivate, isEGLInitialized));
}

bool stopVideoRecord_stub()
{
    return true;
}
void stopStreaming_stub()
{
}

TEST_F(WaylandIntegrationTest, stopStreaming)
{
    stub.set(ADDR(WaylandIntegrationPrivate, stopVideoRecord), stopVideoRecord_stub);
    stub.set(ADDR(WaylandIntegrationPrivate, stopStreaming), stopStreaming_stub);
    stopStreaming();
    stub.reset(ADDR(WaylandIntegrationPrivate, stopVideoRecord));
    stub.reset(ADDR(WaylandIntegrationPrivate, stopStreaming));
}

QMap<quint32, WaylandOutput> screens_stub()
{
    QMap<quint32, WaylandOutput> map;
    WaylandOutput output ;
    output.setWaylandOutputName(9527);
    map[1] = output;
    return map;
}
TEST_F(WaylandIntegrationTest, screens)
{
    stub.set(ADDR(WaylandIntegrationPrivate, screens), screens_stub);
    screens();
    stub.reset(ADDR(WaylandIntegrationPrivate, screens));
}
