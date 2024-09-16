// SPDX-FileCopyrightText: 2011 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "memoryprofile.h"
#include "ddlog.h"
#include <QDebug>
#include <QFile>

#define PROC_MEM_INFOI_PATH "/proc/meminfo"
using namespace DDLog;
MemoryProfile::MemoryProfile(QObject *parent)
    : QObject(parent), mMemUsage(0)
{
}

double MemoryProfile::updateSystemMemoryUsage()
{
    // 返回值，内存占用率
    double memUsage = 0;

    QFile file(PROC_MEM_INFOI_PATH);
    if (file.exists() && file.open(QFile::ReadOnly)) {
        // 计算总的内存占用率，只需要读取前3行数据
        QByteArray lineData1 = file.readLine();
        QByteArray lineData2 = file.readLine();
        QByteArray lineData3 = file.readLine();
        file.close();
        // 数据样例
        // MemTotal:       16346064 kB
        // MemFree:         1455488 kB
        // MemAvailable:    5931304 kB
        if (lineData1.size() == 0 || lineData2.size() == 0 || lineData3.size() == 0) {
            qCWarning(app) << QString(" read %1 file fail !").arg(PROC_MEM_INFOI_PATH) << lineData1 << lineData2 << lineData3;
            return memUsage;
        }

        // 数据提取
        QStringList list1 = QString(lineData1).split(" ", QString::SkipEmptyParts);
        QStringList list2 = QString(lineData2).split(" ", QString::SkipEmptyParts);
        QStringList list3 = QString(lineData3).split(" ", QString::SkipEmptyParts);

        if (list1.size() < 3 || list2.size() < 3 || list3.size() < 3) {
            qCWarning(app) << QString(" parse %1 file fail !").arg(PROC_MEM_INFOI_PATH) << list1 << list2 << list3;
            return memUsage;
        }

        QMap<QString, int> memDataMap;
        memDataMap[list1.at(0)] = list1.at(1).toInt();
        memDataMap[list2.at(0)] = list2.at(1).toInt();
        memDataMap[list3.at(0)] = list3.at(1).toInt();

        // 为返回值赋值，计算内存占用率
        if (memDataMap.contains("MemTotal:") && memDataMap.contains("MemAvailable:") && memDataMap["MemTotal:"] != 0) {
            memUsage = (memDataMap["MemTotal:"] - memDataMap["MemAvailable:"]) * 100.0 / memDataMap["MemTotal:"];
            mMemUsage = memUsage;
        } else {
            qCWarning(app) << QString(" extract mem data fail !") << memDataMap;
        }
    } else {
        qCWarning(app) << QString(" file %1 open fail !").arg(PROC_MEM_INFOI_PATH);
    }

    return memUsage;
}

double MemoryProfile::getMemUsage()
{
    return mMemUsage;
}
