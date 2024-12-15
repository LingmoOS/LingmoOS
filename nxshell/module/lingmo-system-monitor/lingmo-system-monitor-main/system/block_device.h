// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEVICE_H
#define BLOCK_DEVICE_H

#include "private/block_device_p.h"

#include <QSharedDataPointer>
#define MAX_NAME_LEN 128
#define PROC_PATH_DISK      "/proc/diskstats"
#define SYSFS_PATH_BLOCK    "/sys/block"
#define SYSFS_PATH_MODEL    "/sys/block/%1/device/model"
#define SYSFS_PATH_SIZE     "/sys/block/%1/size"

namespace core {
namespace system {

class BlockDevice
{
public:
    BlockDevice();
    BlockDevice(const BlockDevice &other);
    BlockDevice &operator=(const BlockDevice &rhs);
    virtual ~BlockDevice();

    QByteArray deviceName() const;
    QString model() const;
    qulonglong capacity() const;
    qulonglong blocksRead() const;
    qulonglong bytesRead() const;
    qreal sectorsReadPerSecond() const;
    qreal readRequestIssuedPerSecond() const;
    qreal readRequestMergedPerSecond() const;
    qreal readRequestMergedPercent() const;

    qreal percentUtilization() const;
    qulonglong transferPerSecond() const;
    qulonglong blocksWritten() const;
    qulonglong bytesWritten() const;
    qreal sectorsWrittenPerSecond() const;
    qreal writeRequestIssuedPerSecond() const;
    qreal writeRequestMergedPerSecond() const;
    qreal writeRequestMergedPercent() const;
    quint64  readIssuer() const;// 获取读完成次数
    quint64  writeComplete() const; // 获取写完成次数
    quint64  readMerged() const; //  获得合并读完成次数
    quint64  writeMerged() const; // 获取合并写完成次数
    quint64  readSpeed() const; // 获取读速度
    quint64  writeSpeed() const; // 获取写速度

    void setDeviceName(const QByteArray &deviceName);

public:
    void readDeviceInfo();
    void readDeviceModel();
    quint64 readDeviceSize(const QString &deviceName);
    void calcDiskIoStates(const QStringList &diskInfo);

private:
    QSharedDataPointer<BlockDevicePrivate> d;
    qint64 m_time_sec{0};
    timeval timevalList[2] = {timeval{0, 0}, timeval{0, 0}};
};

inline QByteArray BlockDevice::deviceName() const
{
    return d->name;
}

inline QString BlockDevice::model() const
{
    return d->model;
}

inline qulonglong BlockDevice::capacity() const
{
    return d->capacity;
}

inline qulonglong BlockDevice::blocksRead() const
{
    return d->blk_read;
}

inline qulonglong BlockDevice::bytesRead() const
{
    return d->bytes_read;
}

inline qreal BlockDevice::sectorsReadPerSecond() const
{
    return d->rsec_ps;
}

inline qreal BlockDevice::readRequestIssuedPerSecond() const
{
    return d->r_ps;
}

inline qreal BlockDevice::readRequestMergedPerSecond() const
{
    return d->rrqm_ps;
}

inline qreal BlockDevice::readRequestMergedPercent() const
{
    return d->p_rrqm;
}

inline qreal BlockDevice::percentUtilization() const
{
    return d->p_util;
}

inline qulonglong BlockDevice::transferPerSecond() const
{
    return d->tps;
}

inline qulonglong BlockDevice::blocksWritten() const
{
    return d->blk_wrtn;
}

inline qulonglong BlockDevice::bytesWritten() const
{
    return d->bytes_wrtn;
}

inline qreal BlockDevice::sectorsWrittenPerSecond() const
{
    return d->wsec_ps;
}

inline qreal BlockDevice::writeRequestIssuedPerSecond() const
{
    return d->w_ps;
}

inline qreal BlockDevice::writeRequestMergedPerSecond() const
{
    return d->wrqm_ps;
}

inline qreal BlockDevice::writeRequestMergedPercent() const
{
    return d->p_wrqm;
}

inline quint64  BlockDevice::readIssuer() const
{
    return d->read_iss;
}
inline quint64  BlockDevice::writeComplete() const
{
    return d->write_com;
}
inline quint64  BlockDevice::readMerged() const
{
    return d->read_merged;
}

inline quint64  BlockDevice::writeMerged() const
{
    return d->write_merged;
}
inline quint64  BlockDevice::readSpeed() const
{
    return d->read_speed;
}
inline quint64  BlockDevice::writeSpeed() const
{
    return d->wirte_speed;
}



} // namespace system
} // namespace core

#endif // BLOCK_DEVICE_H
