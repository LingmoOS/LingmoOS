// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "testdisomaster.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QMetaType>

using namespace DISOMasterNS;

TestSignalReceiver::TestSignalReceiver(DISOMaster *_d, TestDISOMaster *parent) : QObject(parent), p(parent), d(_d)
{
}
void TestSignalReceiver::updateJobStatus(DISOMaster::JobStatus status, int progress)
{
    fprintf(stderr, "status update: %d %d, speed: %s\n", status, progress, d->getCurrentSpeed().toUtf8().data());
    p->st = status;
    p->p = progress;
}

TestDISOMaster::TestDISOMaster(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<DISOMaster::JobStatus>(QT_STRINGIFY(DISOMaster::JobStatus));
}

void TestDISOMaster::test_getDevice()
{
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DEVICE"));
    const QString dev = QString(qgetenv("DISOMASTERTEST_DEVICE"));
    DISOMaster* x=new DISOMaster;
    x->acquireDevice(dev);
    x->getDeviceProperty();
    x->releaseDevice();
    DeviceProperty dp = x->getDevicePropertyCached(dev);
    fprintf(stderr, "data: %llu, avail: %llu\n", dp.data, dp.avail);
    QVERIFY(dp.devid != "");
    delete x;
}

void TestDISOMaster::test_writeFiles()
{
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DEVICE"));
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DATAPATH"));
    const QString dev = QString(qgetenv("DISOMASTERTEST_DEVICE"));
    const QString path = QString(qgetenv("DISOMASTERTEST_DATAPATH"));

    st = DISOMaster::JobStatus::Idle;
    DISOMaster *x = new DISOMaster;
    TestSignalReceiver *r = new TestSignalReceiver(x, this);
    connect(x, &DISOMaster::jobStatusChanged, r, &TestSignalReceiver::updateJobStatus);

    QFuture<void> f = QtConcurrent::run([=] {
        x->acquireDevice(dev);
        QHash<QUrl, QUrl> files{
            {QUrl(path), QUrl("/")}
        };
        x->stageFiles(files);
        x->commit();
        x->releaseDevice();
    });

    QTRY_VERIFY_WITH_TIMEOUT(st == DISOMaster::JobStatus::Finished, 120000);
    f.waitForFinished();
    delete r;
    delete x;
}

void TestDISOMaster::test_erase()
{
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DEVICE"));
    const QString dev = QString(qgetenv("DISOMASTERTEST_DEVICE"));

    st = DISOMaster::JobStatus::Idle;
    DISOMaster *x = new DISOMaster;
    TestSignalReceiver *r = new TestSignalReceiver(x, this);
    connect(x, &DISOMaster::jobStatusChanged, r, &TestSignalReceiver::updateJobStatus);

    QFuture<void> f = QtConcurrent::run([=] {
        x->acquireDevice(dev);
        x->erase();
        x->releaseDevice();
    });

    QTRY_VERIFY_WITH_TIMEOUT(st == DISOMaster::JobStatus::Finished, 60000);
    f.waitForFinished();
    delete r;
    delete x;
}

void TestDISOMaster::test_isoWrite()
{
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_ISOFILE"));
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DEVICE"));
    const QString dev = QString(qgetenv("DISOMASTERTEST_DEVICE"));
    const QString iso = QString(qgetenv("DISOMASTERTEST_ISOFILE"));

    st = DISOMaster::JobStatus::Idle;
    DISOMaster *x = new DISOMaster;
    TestSignalReceiver *r = new TestSignalReceiver(x, this);
    connect(x, &DISOMaster::jobStatusChanged, r, &TestSignalReceiver::updateJobStatus);

    QFuture<void> f = QtConcurrent::run([=] {
        x->acquireDevice(dev);
        x->writeISO(iso);
        x->releaseDevice();
    });

    QTRY_VERIFY_WITH_TIMEOUT(st == DISOMaster::JobStatus::Finished, 750000);
    f.waitForFinished();
    delete r;
    delete x;
}

void TestDISOMaster::test_checkMedia()
{
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DEVICE"));
    const QString dev = QString(qgetenv("DISOMASTERTEST_DEVICE"));
    st = DISOMaster::JobStatus::Idle;
    DISOMaster *x = new DISOMaster;
    TestSignalReceiver *r = new TestSignalReceiver(x, this);
    connect(x, &DISOMaster::jobStatusChanged, r, &TestSignalReceiver::updateJobStatus);

    QFuture<void> f = QtConcurrent::run([=] {
        double good, slow, bad;
        x->acquireDevice(dev);
        x->checkmedia(&good, &slow, &bad);
        x->releaseDevice();
    });

    QTRY_VERIFY_WITH_TIMEOUT(st == DISOMaster::JobStatus::Finished, 300000);
    f.waitForFinished();
    delete r;
    delete x;
}

void TestDISOMaster::test_dumpISO()
{
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DEVICE"));
    Q_ASSUME(qEnvironmentVariableIsSet("DISOMASTERTEST_DEVICE"));
    const QString dev = QString(qgetenv("DISOMASTERTEST_DEVICE"));
    const QString iso = QString(qgetenv("DISOMASTERTEST_ISOFILE"));

    DISOMaster *x = new DISOMaster;
    TestSignalReceiver *r = new TestSignalReceiver(x, this);
    connect(x, &DISOMaster::jobStatusChanged, r, &TestSignalReceiver::updateJobStatus);

    QFuture<void> f = QtConcurrent::run([=] {
        x->acquireDevice(dev);
        x->dumpISO(QUrl::fromLocalFile(iso));
        x->releaseDevice();
    });

    QTRY_VERIFY_WITH_TIMEOUT(st == DISOMaster::JobStatus::Finished, 300000);
    f.waitForFinished();
    delete r;
    delete x;
}

QTEST_MAIN(TestDISOMaster)
