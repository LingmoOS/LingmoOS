// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef JOBMANAGER1SERVICE_H
#define JOBMANAGER1SERVICE_H

#include <QObject>
#include <QMap>
#include <QDBusObjectPath>
#include <QSharedPointer>
#include <QMutex>
#include <QMutexLocker>
#include <QtConcurrent>
#include <QDBusError>
#include <QFuture>
#include <QUuid>
#include "global.h"
#include "dbus/jobadaptor.h"

class ApplicationManager1Service;

struct LaunchTask
{
    LaunchTask() = default;
    ~LaunchTask() = default;
    LaunchTask(const LaunchTask &) = default;
    LaunchTask(LaunchTask &&) = default;
    LaunchTask &operator=(const LaunchTask &) = default;
    LaunchTask &operator=(LaunchTask &&) = default;
    explicit operator bool() const { return !LaunchBin.isEmpty() and !command.isEmpty(); }
    QString LaunchBin;
    QStringList command;
    QVariantList Resources;
};

Q_DECLARE_METATYPE(LaunchTask)

class JobManager1Service final : public QObject
{
    Q_OBJECT
public:
    JobManager1Service(const JobManager1Service &) = delete;
    JobManager1Service(JobManager1Service &&) = delete;
    JobManager1Service &operator=(const JobManager1Service &) = delete;
    JobManager1Service &operator=(JobManager1Service &&) = delete;

    ~JobManager1Service() override;
    template <typename F>
    QDBusObjectPath addJob(const QString &source, F func, QVariantList args)
    {
        static_assert(std::is_invocable_v<F, QVariant>, "param type must be QVariant.");

        QString objectPath =
            QString{"%1/%2"}.arg(DDEApplicationManager1JobManager1ObjectPath).arg(QUuid::createUuid().toString(QUuid::Id128));
        QFuture<QVariantList> future = QtConcurrent::mappedReduced(std::move(args),
                                                                   func,
                                                                   qOverload<QVariantList::parameter_type>(&QVariantList::append),
                                                                   QVariantList{},
                                                                   QtConcurrent::ReduceOption::OrderedReduce);
        QSharedPointer<JobService> job{new (std::nothrow) JobService{future}};
        if (job == nullptr) {
            qCritical() << "couldn't new JobService.";
            future.cancel();
            return {};
        }

        auto *ptr = job.data();
        auto *adaptor = new (std::nothrow) JobAdaptor(ptr);
        if (adaptor == nullptr or !registerObjectToDBus(ptr, objectPath, JobInterface)) {
            qCritical() << "can't register job to dbus.";
            future.cancel();
            return {};
        }

        auto path = QDBusObjectPath{objectPath};
        {
            QMutexLocker locker{&m_mutex};
            m_jobs.insert(path, job);  // Insertion is always successful
        }
        emit JobNew(path, QDBusObjectPath{source});

        auto emitRemove = [this, job, path, future](QVariantList value) {
            if (!removeOneJob(path)) {
                return value;
            }

            QString result{job->status()};
            for (const auto &val : future.result()) {
                if (val.metaType().id() == QMetaType::fromType<QDBusError>().id()) {
                    result = "failed";
                }
                break;
            }
            emit JobRemoved(path, result, future.result());
            return value;
        };

        future.then(this, emitRemove);
        return path;
    }

Q_SIGNALS:
    void JobNew(const QDBusObjectPath &job, const QDBusObjectPath &source);
    void JobRemoved(const QDBusObjectPath &job, const QString &status, const QVariantList &result);

private:
    bool removeOneJob(const QDBusObjectPath &path);
    friend class ApplicationManager1Service;
    explicit JobManager1Service(ApplicationManager1Service *parent);
    QMutex m_mutex;
    QMap<QDBusObjectPath, QSharedPointer<JobService>> m_jobs;
    ApplicationManager1Service *m_parent{nullptr};
};

#endif
