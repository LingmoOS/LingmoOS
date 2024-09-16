// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ddlog.h"
#include "cpuprofile.h"
#include <QFile>
#include <QDebug>
using namespace DDLog;

#define PROC_CPU_STAT_PATH "/proc/stat"
#define PROC_CPU_INFO_PATH "/proc/cpuinfo"

CpuProfile::CpuProfile(QObject *parent)
    : QObject(parent), mCpuUsage(0.0)
{
    // mLastCpuStat用于记录Cpu状态
    // 各项数值是开机后各项工作的时间片总数
    // 初始化mLastCpuStat
    mLastCpuStat["user"] = 0;
    mLastCpuStat["nice"] = 0;
    mLastCpuStat["sys"] = 0;
    mLastCpuStat["idle"] = 0;
    mLastCpuStat["iowait"] = 0;
    mLastCpuStat["hardqirq"] = 0;
    mLastCpuStat["softirq"] = 0;
    mLastCpuStat["steal"] = 0;
    mLastCpuStat["guest"] = 0;
    mLastCpuStat["guest_nice"] = 0;
    // total is sum of above items
    mLastCpuStat["total"] = 0;

    // 更新数据
    updateSystemCpuUsage();
}

double CpuProfile::updateSystemCpuUsage()
{
    // 返回值，Cpu占用率
    double cpuUsage = 0.0;

    QFile file(PROC_CPU_STAT_PATH);
    if (file.exists() && file.open(QFile::ReadOnly)) {
        // 计算总的Cpu占用率，只需要读取第一行数据
        QByteArray lineData = file.readLine();
        file.close();
        // 样例数据 ： cpu  7048360 4246 3733400 801045435 846386 0 929664 0 0 0
        //         |user|nice|sys|idle|iowait|hardqirq|softirq|steal|guest|guest_nice|

        // 分割行数据
        QStringList cpuStatus = QString(lineData).split(" ", QString::SkipEmptyParts);

        // CPU状态应包含10个数据片段，有效数据 1-10，位置0不使用
        if (cpuStatus.size() < 11) {
            return cpuUsage;
        }

        // 构建数据map，便于后期数据计算方式需求变更
        QMap<QString, int> curCpuStat;
        {
            curCpuStat["user"] = cpuStatus.at(1).toInt();
            curCpuStat["nice"] = cpuStatus.at(2).toInt();
            curCpuStat["sys"] = cpuStatus.at(3).toInt();
            curCpuStat["idle"] = cpuStatus.at(4).toInt();
            curCpuStat["iowait"] = cpuStatus.at(5).toInt();
            curCpuStat["hardqirq"] = cpuStatus.at(6).toInt();
            curCpuStat["softirq"] = cpuStatus.at(7).toInt();
            curCpuStat["steal"] = cpuStatus.at(8).toInt();
            curCpuStat["guest"] = cpuStatus.at(9).toInt();
            curCpuStat["guest_nice"] = cpuStatus.at(10).toInt();
        }

        // 计算当前总的Cpu时间片
        int curTotalCpu = 0;
        for (int i = 1; i <= 10; i++) {
            curTotalCpu = curTotalCpu + cpuStatus.at(i).toInt();
        }
        curCpuStat["total"] = curTotalCpu;

        // 计算cpu占用, 使用double精度计算
        // 通过对当前系统Cpu时间片使用情况和上一次获取的系统Cpu时间片使用情况，来计算上一个时间段内的Cpu使用情况
        double calcCpuTotal = curCpuStat["total"] - mLastCpuStat["total"];
        double calcCpuIdle =
                (curCpuStat["idle"] + curCpuStat["iowait"]) - (mLastCpuStat["idle"] + mLastCpuStat["iowait"]);

        if (calcCpuTotal == 0.0) {
            qCWarning(app) << " cpu total usage calc result equal 0 ! cpu stat [" << curCpuStat << "]";
            return cpuUsage;
        }
        // 上一个时间段内的Cpu使用情况
        cpuUsage = (calcCpuTotal - calcCpuIdle) * 100.0 / calcCpuTotal;

        // 更新Cpu占用率
        mCpuUsage = cpuUsage;

        // 更新上一次CPU状态
        mLastCpuStat = curCpuStat;
    } else {
        qCWarning(app) << QString(" file %1 open fail !").arg(PROC_CPU_STAT_PATH);
    }

    return cpuUsage;
}

QMap<QString, int> CpuProfile::cpuStat()
{
    return mLastCpuStat;
}

double CpuProfile::getCpuUsage()
{
    return mCpuUsage;
}
