// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DDiskJob>
#include "ddiskjob_p.h"

DMOUNT_USE_NAMESPACE

DDiskJob::DDiskJob(const QString &path, QObject *parent)
    : QObject { parent }, d_ptr { new DDiskJobPrivate(this) }
{
    Q_D(DDiskJob);
    d->iface = new OrgFreedesktopUDisks2JobInterface(kUDisks2Service, path, QDBusConnection::systemBus(), this);
    QDBusConnection::systemBus().connect(kUDisks2Service, d->iface->path(), "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged", this, SLOT(onPropertiesChanged(const QString &, const QVariantMap &)));
    connect(d->iface, &OrgFreedesktopUDisks2JobInterface::Completed, this, &DDiskJob::completed);
}

DDiskJob::~DDiskJob()
{
}

QString DDiskJob::path() const
{
    Q_D(const DDiskJob);
    return d->iface->path();
}

QStringList DDiskJob::objects() const
{
    Q_D(const DDiskJob);
    QStringList ret;
    auto &&objs = d->iface->objects();
    for (const auto &obj : objs)
        ret << obj.path();
    return ret;
}

bool DDiskJob::cancelable() const
{
    Q_D(const DDiskJob);
    return d->iface->cancelable();
}

bool DDiskJob::progressValid() const
{
    Q_D(const DDiskJob);
    return d->iface->progressValid();
}

double DDiskJob::progress() const
{
    Q_D(const DDiskJob);
    return d->iface->progress();
}

QString DDiskJob::operation() const
{
    Q_D(const DDiskJob);
    return d->iface->operation();
}

quint32 DDiskJob::startedByUid() const
{
    Q_D(const DDiskJob);
    return d->iface->startedByUID();
}

quint64 DDiskJob::bytes() const
{
    Q_D(const DDiskJob);
    return d->iface->bytes();
}

quint64 DDiskJob::expectedEndTime() const
{
    Q_D(const DDiskJob);
    return d->iface->expectedEndTime();
}

quint64 DDiskJob::rate() const
{
    Q_D(const DDiskJob);
    return d->iface->rate();
}

quint64 DDiskJob::startTime() const
{
    Q_D(const DDiskJob);
    return d->iface->startTime();
}

void DDiskJob::cancel(const QVariantMap &options)
{
    Q_D(DDiskJob);
    d->iface->Cancel(options).waitForFinished();
}
