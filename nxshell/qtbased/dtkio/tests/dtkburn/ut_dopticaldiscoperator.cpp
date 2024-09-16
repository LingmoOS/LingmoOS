// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include <DOpticalDiscOperator>
#include <DOpticalDiscManager>
#include <DOpticalDiscInfo>

#include <QObject>

#include <gtest/gtest.h>
#include <stubext.h>

#include "dopticaldiscoperator_p.h"
#include "dxorrisoengine.h"
#include "dudfburnengine.h"

DBURN_USE_NAMESPACE
DCORE_USE_NAMESPACE

class TestDOpticalDiscOperator : public testing::Test
{
public:
    void SetUp() override
    {
        m_stub.set_lamda(ADDR(DXorrisoEngine, acquireDevice), [](DXorrisoEngine *, const QString &dev) { __DBG_STUB_INVOKE__
            return true; });
        m_stub.set_lamda(ADDR(DXorrisoEngine, releaseDevice), [] { __DBG_STUB_INVOKE__ });
        m_operator = new DOpticalDiscOperator("test");
    }
    void TearDown() override
    {
        delete m_operator;
        m_stub.clear();
    }

    stub_ext::StubExt m_stub;
    DOpticalDiscOperator *m_operator { nullptr };
};

TEST_F(TestDOpticalDiscOperator, private_makeStageFiles)
{
    QString msg;
    auto ret { m_operator->d_ptr->makeStageFiles({}, &msg) };
    EXPECT_FALSE(ret);

    ret = m_operator->d_ptr->makeStageFiles("/tmp", &msg);
    EXPECT_TRUE(ret);
}

TEST_F(TestDOpticalDiscOperator, private_onUDFEngineJobStatusChanged)
{
    // test failed
    {
        auto conn = QObject::connect(m_operator, &DOpticalDiscOperator::jobStatusChanged,
                                     [](JobStatus status, int progress, QString speed, QStringList message) {
                                         Q_UNUSED(speed);
                                         Q_UNUSED(message);
                                         EXPECT_EQ(status, JobStatus::Failed);
                                         EXPECT_EQ(progress, 0);
                                     });

        m_operator->d_ptr->onUDFEngineJobStatusChanged(JobStatus::Failed, 0);
        QObject::disconnect(conn);
    }

    // test success
    {
        QObject parent;
        m_operator->d_ptr->currentEngine = new DUDFBurnEngine(&parent);
        auto conn = QObject::connect(m_operator, &DOpticalDiscOperator::jobStatusChanged,
                                     [](JobStatus status, int progress, QString speed, QStringList message) {
                                         Q_UNUSED(speed);
                                         Q_UNUSED(message);
                                         EXPECT_EQ(status, JobStatus::Finished);
                                         EXPECT_EQ(progress, 100);
                                     });

        m_operator->d_ptr->onUDFEngineJobStatusChanged(JobStatus::Finished, 100);
        QObject::disconnect(conn);
    }
}

TEST_F(TestDOpticalDiscOperator, private_onXorrisoEngineJobStatusChanged)
{
    // test failed
    {
        auto conn = QObject::connect(m_operator, &DOpticalDiscOperator::jobStatusChanged,
                                     [](JobStatus status, int progress, QString speed, QStringList message) {
                                         Q_UNUSED(message);
                                         EXPECT_EQ(status, JobStatus::Failed);
                                         EXPECT_EQ(progress, 0);
                                         EXPECT_EQ(speed, "10.0x");
                                     });
        m_operator->d_ptr->onXorrisoEngineJobStatusChanged(JobStatus::Failed, 0, "10.0x");
        QObject::disconnect(conn);
    }

    // test success
    {
        QObject parent;
        m_operator->d_ptr->currentEngine = new DXorrisoEngine(&parent);
        auto conn = QObject::connect(m_operator, &DOpticalDiscOperator::jobStatusChanged,
                                     [](JobStatus status, int progress, QString speed, QStringList message) {
                                         Q_UNUSED(message);
                                         EXPECT_EQ(status, JobStatus::Finished);
                                         EXPECT_EQ(progress, 100);
                                         EXPECT_EQ(speed, "10.0x");
                                     });

        m_operator->d_ptr->onXorrisoEngineJobStatusChanged(JobStatus::Finished, 100, "10.0x");
        QObject::disconnect(conn);
    }
}

TEST_F(TestDOpticalDiscOperator, burn)
{
    // test failed
    {
        auto ret = m_operator->burn({}, {});
        EXPECT_FALSE(ret.hasValue());
    }

    // test UDF
    {
        m_stub.set_lamda(ADDR(DUDFBurnEngine, doBurn),
                         [](DUDFBurnEngine *, const QString &, const QPair<QString, QString> &, const QString &) {
                             __DBG_STUB_INVOKE__
                             return true;
                         });
        auto ret = m_operator->burn("/tmp", BurnOption::UDF102Supported);
        EXPECT_TRUE(ret);
    }

    // test xorriso
    {
        m_stub.set_lamda(ADDR(DXorrisoEngine, doBurn),
                         [](DXorrisoEngine *, const QPair<QString, QString>, int,
                            QString, DXorrisoEngine::JolietSupported,
                            DXorrisoEngine::RockRageSupported rockRage, DXorrisoEngine::KeepAppendable) {
                             __DBG_STUB_INVOKE__
                             return true;
                         });
        auto ret = m_operator->burn("/tmp", BurnOption::JolietSupported);
        EXPECT_TRUE(ret);
    }
}

TEST_F(TestDOpticalDiscOperator, erase)
{
    m_stub.set_lamda(ADDR(DXorrisoEngine, doErase), []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    auto ret = m_operator->erase();
    EXPECT_TRUE(ret.hasValue());
    EXPECT_TRUE(ret.value());

    m_stub.set_lamda(ADDR(DXorrisoEngine, doErase), []() {
        __DBG_STUB_INVOKE__
        return false;
    });
    ret = m_operator->erase();
    EXPECT_FALSE(ret.hasValue());
}

TEST_F(TestDOpticalDiscOperator, checkmedia)
{
    // unexpected
    {
        m_operator->d_ptr->curDev = "";
        auto ret { m_operator->checkmedia() };
        EXPECT_FALSE(ret.hasValue());
    }

    // doCheckmedia
    {
        // expected
        m_stub.set_lamda(ADDR(DOpticalDiscManager, createOpticalDiscInfo), [](const QString &dev, QObject *parent) {
            __DBG_STUB_INVOKE__
            return new DOpticalDiscInfo(dev, parent);
        });

        m_stub.set_lamda(ADDR(DXorrisoEngine, doCheckmedia), [](DXorrisoEngine *, quint64, double *, double *, double *bad) {
            __DBG_STUB_INVOKE__
            *bad = 0;
            return true;
        });
        auto ret { m_operator->checkmedia() };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());

        // unexpected
        m_stub.set_lamda(ADDR(DXorrisoEngine, doCheckmedia), [](DXorrisoEngine *, quint64, double *, double *, double *bad) {
            __DBG_STUB_INVOKE__
            *bad = 3;
            return true;
        });
        ret = m_operator->checkmedia();
        EXPECT_FALSE(ret.hasValue());

        // unexpected
        m_stub.set_lamda(ADDR(DXorrisoEngine, doCheckmedia), [](DXorrisoEngine *, quint64, double *, double *, double *) {
            __DBG_STUB_INVOKE__
            return false;
        });
        ret = m_operator->checkmedia();
        EXPECT_FALSE(ret.hasValue());
    }
}

TEST_F(TestDOpticalDiscOperator, writeISO)
{
    // unexpected
    {
        auto ret { m_operator->writeISO("") };
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(ADDR(DXorrisoEngine, doWriteISO), [](DXorrisoEngine *, const QString &, int) {
            return true;
        });
        auto ret { m_operator->writeISO("/a/b/c") };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}

TEST_F(TestDOpticalDiscOperator, dumpISO)
{
    // unexpected
    {
        m_operator->d_ptr->curDev = "";
        auto ret { m_operator->dumpISO("") };
        EXPECT_FALSE(ret);
    }

    // expected
    {
        m_stub.set_lamda(ADDR(DXorrisoEngine, doDumpISO), [](DXorrisoEngine *, quint64, const QString &) {
            return true;
        });
        m_operator->d_ptr->curDev = "test";
        auto ret { m_operator->dumpISO("/a/b/c") };
        EXPECT_TRUE(ret.hasValue());
        EXPECT_TRUE(ret.value());
    }
}
