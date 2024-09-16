// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/jobmanager1service.h"
#include <gtest/gtest.h>

class TestJobManager : public testing::Test
{
public:
    static void SetUpTestCase() { m_jobManager = new JobManager1Service(nullptr); }

    static void TearDownTestCase() { m_jobManager->deleteLater(); }
    JobManager1Service &service() { return *m_jobManager; }

private:
    static inline JobManager1Service *m_jobManager{nullptr};
};

TEST_F(TestJobManager, addJob)
{
    QDBusObjectPath sourcePath{"/org/deepin/Test1"};
    QVariantList args{{"Application"}, {"Application"}, {"Application"}, {"Application"}};
    auto &manager = service();
    QDBusObjectPath jobPath;
    QObject::connect(&manager, &JobManager1Service::JobNew, [&](const QDBusObjectPath &job, const QDBusObjectPath &source) {
        jobPath = job;
        EXPECT_TRUE(source == sourcePath);
    });
    QObject::connect(&manager,
                     &JobManager1Service::JobRemoved,
                     [&](const QDBusObjectPath &job, const QString &status, const QVariantList &result) {
                         EXPECT_TRUE(jobPath == job);
                         EXPECT_TRUE(status == "finished");
                         EXPECT_TRUE(result.count() == 4);
                         qDebug() << "job was really removed";
                     });

    manager.addJob(
        sourcePath.path(),
        [](auto value) -> QVariant {
            EXPECT_TRUE(value.toString() == "Application");
            return QVariant::fromValue(true);
        },
        args);
    QThreadPool::globalInstance()->waitForDone();
}
