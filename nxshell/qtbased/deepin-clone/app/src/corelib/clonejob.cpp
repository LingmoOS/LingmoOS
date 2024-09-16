// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "clonejob.h"
#include "ddiskinfo.h"
#include "ddevicediskinfo.h"
#include "dfilediskinfo.h"
#include "helper.h"
#ifdef ENABLE_BOOTDOCTOR
#include "bootdoctor.h"
#endif

#include <QDir>
#include <QElapsedTimer>

#include <functional>

CloneJob::CloneJob(QObject *parent)
    : QThread(parent)
    , m_status(Stoped)
{
    connect(this, &QThread::finished, this, [this] {
        setStatus(Stoped);

        if (!m_abort)
            emit finished();
    });
}

CloneJob::~CloneJob()
{
    if (isRunning()) {
        m_abort = true;
        quit();
        wait();
    }
}

bool CloneJob::start(const QString &from, const QString &to)
{
    if (isRunning())
        return false;

    m_abort = false;
    m_from = from;
    m_to = to;
    m_errorString.clear();
    m_progress = 0;
    m_estimateTime = 0;

    QThread::start();

    return true;
}

void CloneJob::abort()
{
    m_abort = true;
}

CloneJob::Status CloneJob::status() const
{
    return m_status;
}

qreal CloneJob::progress() const
{
    return m_progress;
}

int CloneJob::estimateTime() const
{
    return m_estimateTime;
}

QString CloneJob::errorString() const
{
    return m_errorString;
}

typedef std::function<bool(qint64 accomplishBytes, int)> PipeNotifyFunction;

static bool diskInfoPipe(DDiskInfo &from, DDiskInfo &to, DDiskInfo::DataScope scope,
                         int fromIndex = 0, int toIndex = 0, QString *error = 0, PipeNotifyFunction *notify = 0)
{
    bool ok = false;
    char block[Global::bufferSize];
    QElapsedTimer elapsedTimer;
    int speed = 10000000;
    qint64 total_size = 0;

    if (!from.beginScope(scope, DDiskInfo::Read, fromIndex)) {
        if (error)
            *error = from.errorString();

        dCDebug("BeginScope failed, scope: %d, index: %d, mode: Read", scope, fromIndex);

        goto exit;
    }

    if (!to.beginScope(scope, DDiskInfo::Write, toIndex)) {
        if (error)
            *error = to.errorString();

        dCDebug("BeginScope failed, scope: %d, index: %d, mode: Write", scope, toIndex);

        goto exit;
    }

    elapsedTimer.start();

    while (!from.atEnd()) {
        qint64 read_size = from.read(block, Global::bufferSize);

        if (read_size <= 0) {
            if (error)
                *error = from.errorString();

            dCError("Reading data from \"%1\" failed, error: %2", qPrintable(from.filePath()), qPrintable(from.errorString()));

            goto exit;
        }

        qint64 write_size = to.write(block, read_size);

        if (write_size < read_size) {
            if (error)
                *error = QCoreApplication::translate("CloneJob", "Writing data to %1 failed, expected write size: %2 — only %3 written, error: %4").arg(to.filePath()).arg(read_size).arg(write_size).arg(to.errorString());

            goto exit;
        }

        if (notify)
            if (!(*notify)(write_size, speed))
                return false;

        total_size += write_size;

        if (elapsedTimer.elapsed() > 0)
            speed = total_size / (qreal)elapsedTimer.elapsed() * 1000;
    }

    ok = true;

exit:

    if (!from.endScope()) {
        if (error)
            *error = from.errorString();

        ok = false;
    }

    if (!to.endScope()) {
        if (error)
            *error = to.errorString();

        ok = false;
    }

    return ok;
}

void CloneJob::run()
{
    QElapsedTimer timer;

    timer.start();

    setStatus(Started);

    dCInfo("Clone job start, source: %s, target: %s", qPrintable(m_from), qPrintable(m_to));

    if (!QFile::exists(m_from)) {
        setErrorString(tr("%1 not exist").arg(m_from));

        return;
    }

    if (Helper::isBlockSpecialFile(m_from)) {
        dCDebug("Refresh device: %s", qPrintable(m_from));

        Helper::refreshSystemPartList(m_from);
    }

    DDiskInfo from_info = DDiskInfo::getInfo(m_from);

    if (!from_info) {
        setErrorString(tr("%1 invalid or not exist").arg(m_from));

        return;
    }

    if (Helper::isBlockSpecialFile(m_to)) {
        dCDebug("Refresh device: %s", qPrintable(m_to));

        Helper::refreshSystemPartList(m_to);

        if (Helper::isDiskDevice(m_from) != Helper::isDiskDevice(m_to)) {
            if (Helper::isDiskDevice(m_from) && from_info.hasScope(DDiskInfo::PartitionTable)) {
                setErrorString(tr("Disk only can be cloned to disk"));

                return;
            }
        }
    } else if (Global::isOverride) {
        QFile file(m_to);

        if (!file.resize(0)) {
            dCWarning("Failed do override file: %s", qPrintable(m_to));
        }
    }

    DDiskInfo to_info = DDiskInfo::getInfo(m_to);

    if (!to_info) {
        setErrorString(tr("%1 invalid or not exist").arg(m_to));

        return;
    }

    if (to_info.totalSize() < from_info.maxReadableDataSize()) {
        setErrorString(tr("%1 total capacity is less than maximum readable data on %2").arg(m_to).arg(Helper::sizeDisplay(from_info.maxReadableDataSize())).arg(m_from));

        return;
    }

    qint64 from_info_total_data_size = from_info.totalReadableDataSize();
    qint64 have_been_written = 0;

    dCDebug("The total amount of data to be backed up: %s", qPrintable(Helper::sizeDisplay(from_info_total_data_size)));

    if (to_info.totalWritableDataSize() < from_info_total_data_size) {
        dCDebug("%s write space is less than %s total capacity", qPrintable(m_to), qPrintable(m_from));

        if (!to_info.setTotalWritableDataSize(from_info_total_data_size)) {
            setErrorString(tr("Failed to change %1 size, please check the free space on target disk").arg(m_to));

            return;
        }
    }

    qint8 progress = 0;

    PipeNotifyFunction print_fun = [from_info_total_data_size, &have_been_written, &progress, this] (qint64 accomplishBytes, int speed) {
        if (m_abort)
            return false;

        have_been_written += accomplishBytes;

        if (qFuzzyCompare(m_progress, 0.99))
            return true;

        m_progress = ((have_been_written / 1000000.0) / (from_info_total_data_size  / 1000000.0));
        m_progress = qMin(m_progress, 0.99);
        m_estimateTime = from_info_total_data_size / (qreal)speed * (1 - m_progress);

        if (Global::isTUIMode) {
            printf("\033[A");
            fflush(stdout);
            printf("----%lld bytes of data have been written, total progress: %f----\n", have_been_written, m_progress * 100);
        } else if (progress != (int)(m_progress * 100)) {
            progress = m_progress * 100;
            dCDebug("----%lld bytes of data have been written, total progress: %d----", have_been_written, progress);
        }

        emit progressChanged(m_progress);

        return true;
    };

    auto call_disk_pipe = [&print_fun, this, &from_info, &to_info] (DDiskInfo::DataScope scope, int fromIndex = 0, int toIndex = 0) {
        QString error;

        printf("\n");

        if (!diskInfoPipe(from_info, to_info, scope, fromIndex, toIndex, &error, &print_fun)) {
            setErrorString(error);
            printf("\n");

            return false;
        }

        printf("\n");

        return true;
    };

    if (from_info.hasScope(DDiskInfo::Headgear)) {
        setStatus(Clone_Headgear);

        dCInfo("begin clone headgear......................\n");

        if (!call_disk_pipe(DDiskInfo::Headgear)) {
            dCDebug("failed!!!");
            setStatus(Failed);

            return;
        }
    }

    if (from_info.hasScope(DDiskInfo::PartitionTable)) {
        setStatus(Clone_PartitionTable);

        dCInfo("begin clone partition table......................\n");

        if (!call_disk_pipe(DDiskInfo::PartitionTable)) {
            dCDebug("failed!!!");
            setStatus(Failed);

            return;
        }
    }

    const QList<DPartInfo> &list = from_info.childrenPartList();

    for (const DPartInfo &info : list) {
        if (!from_info.hasScope(DDiskInfo::Partition, DDiskInfo::Read, info.indexNumber()))
            continue;

        setStatus(Clone_Partition);

        dCInfo("begin clone partition, index: %d......................\n", info.indexNumber());

        if (!call_disk_pipe(DDiskInfo::Partition, info.indexNumber(), info.indexNumber())) {
            dCDebug("failed!!!");
            setStatus(Failed);

            return;
        }

        Helper::processExec("fsck", {"-f", "-y", QString::number(info.indexNumber())});

        if (info.fileSystemType() == DPartInfo::EXT4
                || info.fileSystemType() == DPartInfo::EXT3
                || info.fileSystemType() == DPartInfo::EXT2) {
            Helper::processExec("resize2fs", {"-p", "-f", QString::number(info.indexNumber())});
        }
    }

    if (from_info.hasScope(DDiskInfo::JsonInfo) && to_info.hasScope(DDiskInfo::JsonInfo, DDiskInfo::Write)) {
        setStatus(Save_Info);

        dCInfo("begin clone json info\n");

        if (!call_disk_pipe(DDiskInfo::JsonInfo)) {
            dCDebug("failed!!!");
            setStatus(Failed);

            return;
        }
    }

    m_estimateTime = 0;

    if (!m_abort) {
        dCDebug("clone finished!");

#ifdef ENABLE_BOOTDOCTOR
        if (Global::fixBoot
                && to_info.type() == DDiskInfo::Part
                && to_info.ptType() != DDiskInfo::Unknow
                && from_info.childrenPartList().first().isDeepinSystemRoot()
                && from_info.type() == DDiskInfo::Part) {
            dCInfo("Try fix boot for \"%s\"", qPrintable(m_to));
            setStatus(Fix_Boot);

            if (!BootDoctor::fix(m_to)) {
                setErrorString(BootDoctor::errorString());

                dCError("Failed fix boot");
            }
        }
#endif

        emit progressChanged(1.0);
    }

    dCInfo("Total time: %s, Total data: %s", qPrintable(Helper::secondsToString(timer.elapsed() / 1000)), qPrintable(Helper::sizeDisplay(have_been_written)));
}

void CloneJob::setStatus(CloneJob::Status s)
{
    if (s == m_status)
        return;

    m_status = s;

    emit statusChanged(s);
}

void CloneJob::setErrorString(const QString &error)
{
    m_errorString = error;

    dCError(error);

    emit failed(error);
}
