// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/jobservice.h"
#include <gtest/gtest.h>
#include <QFuture>
#include <QPromise>
#include <thread>
#include <chrono>
#include <QMutex>

using namespace std::chrono_literals;

class TestJob : public testing::Test
{
public:
    void SetUp() override
    {
        if (!(QAtomicInt::isFetchAndAddNative() or QAtomicInt::isFetchAndAddWaitFree())) {
            GTEST_SKIP() << "Current platform doesn't support native lockFree or waitFree fetchAdd.";
        }
        QPromise<QVariantList> promise;
        m_jobService = new JobService{promise.future()};

        m_thread = QThread::create(
            [this](QPromise<QVariantList> promise) {
                while (true) {
                    if (m_atomic.loadAcquire() == 1) {  // 1
                        break;
                    }
                }

                promise.start();
                m_atomic.fetchAndAddAcquire(1);  // 2

                while (true) {
                    if (m_atomic.loadAcquire() == 3) {  // 3
                        break;
                    }
                }

                if (promise.isCanceled()) {
                    m_atomic.fetchAndAddAcquire(1);  // 4
                    return;
                }

                promise.suspendIfRequested();

                std::size_t x{0};
                for (std::size_t i = 0; i < 20000; ++i) {
                    x += 1;
                }

                promise.addResult(QVariantList{QVariant::fromValue<std::size_t>(x)});
                promise.finish();
            },
            std::move(promise));
    }

    void TearDown() override
    {
        m_thread->quit();
        m_thread->deleteLater();
        m_thread = nullptr;

        m_jobService->deleteLater();
        m_jobService = nullptr;
    }

    QAtomicInt m_atomic{0};
    QThread *m_thread{nullptr};
    JobService *m_jobService{nullptr};
};

TEST_F(TestJob, cancelJob)
{
    EXPECT_EQ(m_jobService->status().toStdString(), std::string{"pending"});
    m_thread->start();
    m_atomic.fetchAndAddAcquire(1);  // 1

    while (true) {
        if (m_atomic.loadAcquire() == 2) {  // 2
            break;
        }
    }
    EXPECT_EQ(m_jobService->status().toStdString(), std::string{"running"});
    m_jobService->Cancel();
    m_atomic.fetchAndAddAcquire(1);  // 3

    while (true) {
        if (m_atomic.loadAcquire() == 4) {  // 4
            break;
        }
    }

    qInfo() << "test canceled";
    EXPECT_EQ(m_jobService->status().toStdString(), std::string{"canceled"});
}

TEST_F(TestJob, suspendAndResumeJob)
{
    m_thread->start();
    m_atomic.fetchAndAddAcquire(1);  // 1

    while (true) {
        if (m_atomic.loadAcquire() == 2) {  // 2
            break;
        }
    }
    EXPECT_EQ(m_jobService->status().toStdString(), std::string{"running"});

    m_jobService->Suspend();

    m_atomic.fetchAndAddAcquire(1);  // 3
    const auto &state = m_jobService->status().toStdString();
    EXPECT_TRUE(state == std::string{"suspending"} or state == std::string{"suspended"});

    m_jobService->Resume();

    EXPECT_EQ(m_jobService->status().toStdString(), std::string{"running"});
    EXPECT_TRUE(m_thread->wait());
    EXPECT_EQ(m_jobService->status().toStdString(), std::string{"finished"});

    auto ret = m_jobService->m_job.result();
    EXPECT_FALSE(ret.isEmpty());
    EXPECT_EQ(ret.first().value<std::size_t>(), std::size_t{20000});
}
