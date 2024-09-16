// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BLOCK_DEVICE_P_H
#define BLOCK_DEVICE_P_H

#include <QSharedData>
#include <QDateTime>

namespace core {
namespace system {

class BlockDevice;

class BlockDevicePrivate : public QSharedData
{
public:
    BlockDevicePrivate()
        : QSharedData()
        , name {}
        , model {}
        , read_speed {0}
        , wirte_speed {0}
        , capacity {0}
        , blk_read {0}
        , bytes_read {0}
        , r_ps {.0}
        , rsec_ps {.0}
        , rrqm_ps {.0}
        , p_rrqm {.0}
        , p_util {.0}
        , tps {0}
        , blk_wrtn {0}
        , bytes_wrtn {0}
        , w_ps {.0}
        , wsec_ps {.0}
        , wrqm_ps {.0}
        , p_wrqm {.0}
        , read_iss {0}
        , write_com{0}
        , read_merged{0}
        , write_merged{0}
        , discard_sector{0}
        , _time_Sec{ QDateTime::currentSecsSinceEpoch() }
    {
    }
    BlockDevicePrivate(const BlockDevicePrivate &other)
        : QSharedData(other)
        , name(other.name)
        , model(other.model)
        , read_speed {other.read_speed}
        , wirte_speed {other.wirte_speed}
        , capacity(other.capacity)
        , blk_read(other.blk_read)
        , bytes_read(other.bytes_read)
        , r_ps(other.r_ps)
        , rsec_ps(other.rsec_ps)
        , rrqm_ps(other.rrqm_ps)
        , p_rrqm(other.p_rrqm)
        , p_util(other.p_util)
        , tps(other.tps)
        , blk_wrtn(other.blk_wrtn)
        , bytes_wrtn(other.bytes_wrtn)
        , w_ps(other.w_ps)
        , wsec_ps(other.wsec_ps)
        , wrqm_ps(other.wrqm_ps)
        , p_wrqm(other.p_wrqm)
        , read_iss {other.read_iss}
        , write_com{other.write_com}
        , read_merged{other.read_merged}
        , write_merged{other.write_merged}
        , discard_sector{other.discard_sector}
        , _time_Sec{other._time_Sec}
    {
    }

private:
    QByteArray name; // device name, eg: /dev/sda, /dev/loop0 ...
    QString model; // device model (might be vitual device)
    unsigned long long read_speed;  // 读取速度
    unsigned long long wirte_speed; // 写入速度
    unsigned long long capacity; // device capacity  磁盘总容量
    unsigned long long blk_read; // total number of blocks read 读扇区次数                       1
    unsigned long long bytes_read; // total number of bytes read　读扇区次数　*512
    double r_ps; // read requests completed per second 1  读完成次数 / 时间间隔                    1
    double rsec_ps; // sectors read per second  2读取扇区次数／时间间隔                            1
    double rrqm_ps; // read requests merged per second 3合并读完成次数/ 时间间隔                   1
    double p_rrqm; // percentage of read requests merged together  4-合并读完成次数/读完成次数 % ?

    double p_util; // bandwidth utilization for the device 5- ??????   ?
    unsigned long long tps; // transfers per second 读完成的次数 + 写完成次数
    unsigned long long blk_wrtn; // total number of blocks written  写扇区此数                   1
    unsigned long long bytes_wrtn; // total number of bytes read 写扇区此数 *512
    double w_ps; // write requests completed per second 写完成次数/时间间隔
    double wsec_ps; // sectors written per second 写扇区次数/时间间隔
    double wrqm_ps; // write requests merged per second 合并写完成次数/时间间隔
    double p_wrqm; // percentage of write requests merged together 合并写完成次数/写完成次数 % ?

    unsigned long long read_iss; // 读完成次数
    unsigned long long write_com; // 写完成次数
    unsigned long long read_merged; // 合并读完成次数
    unsigned long long write_merged; // 合并写完成次数
    quint64            discard_sector; // 放弃的扇区

    qint64 _time_Sec;   //记录的时间

    friend class BlockDevice;
};

} // namespace system
} // namespace core

#endif // BLOCK_DEVICE_P_H
