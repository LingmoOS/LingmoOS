// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef JOBSERVICE_H
#define JOBSERVICE_H

#include <QObject>
#include <QFuture>
#include <QVariant>

class JobService : public QObject
{
    Q_OBJECT
public:
    ~JobService() override;
    JobService(const JobService &) = delete;
    JobService(JobService &&) = delete;
    JobService &operator=(const JobService &) = delete;
    JobService &operator=(JobService &&) = delete;

    Q_PROPERTY(QString Status READ status)
    QString status() const;

public Q_SLOTS:
    void Cancel();
    void Suspend();
    void Resume();

private:
    friend class JobManager1Service;
    explicit JobService(const QFuture<QVariantList> &job);
    QFuture<QVariantList> m_job;
};

#endif
