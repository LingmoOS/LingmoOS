// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISKIO_INFO_H
#define DISKIO_INFO_H

#include <QMap>
#include <memory>

namespace core {
namespace system {

struct disk_io_stat {
    unsigned long long total_ios;
    unsigned long long read_ios;            // # of reads completed
    unsigned long long read_sectors;        // # of sectors read
    unsigned long long write_ios;           // # of writes completed
    unsigned long long write_sectors;       // # of sectors written
    unsigned long long discard_ios;         // # of discards completed
    unsigned long long discard_sectors;     // # of sectors discarded
};

class DiskIOInfo
{
    enum StatIndex { kLastStat = 0, kCurrentStat = 1, kStatCount = kCurrentStat + 1 };

public:
    explicit DiskIOInfo();
    virtual ~DiskIOInfo();

    void update();
    qreal diskIoReadBps();
    qreal diskIoWriteBps();

private:
    void readDiskIOStats();
    void calDiskIoStates();

private:
    QMap<QString, std::shared_ptr<disk_io_stat>> m_diskIoStatMap[kStatCount];
    timeval timevalList[kStatCount] = {timeval{0, 0}, timeval{0, 0}};

    qreal m_readBps = 0;
    qreal m_writeBps = 0;
};

} // namespace system
} // namespace core

#endif // DISKIO_INFO_H
