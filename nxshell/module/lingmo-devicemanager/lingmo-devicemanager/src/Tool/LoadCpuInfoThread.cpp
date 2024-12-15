// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LoadCpuInfoThread.h"

#include <QProcess>
#include <QLoggingCategory>

#include "DeviceManager.h"
#include "DeviceCpu.h"

LoadCpuInfoThread::LoadCpuInfoThread()
{
}

void LoadCpuInfoThread::run()
{
    getCpuInfoFromLscpu();
}

void LoadCpuInfoThread::runCmd(QString &info, const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
}

void LoadCpuInfoThread::loadCpuInfo(QMap<QString, QString> &mapInfo, const QString &cmd)
{
    QString cpuInfo;
    runCmd(cpuInfo, cmd);
    getMapInfoFromCmd(cpuInfo, mapInfo, ": ");
}

void LoadCpuInfoThread::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2)
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
    }
}

void LoadCpuInfoThread::getCpuInfoFromLscpu()
{
    // 生成CPU
    const QList<QMap<QString, QString>> &lstCatCpu = DeviceManager::instance()->cmdInfo("lscpu");
    if (lstCatCpu.size() == 0)
        return;
    QMap<QString, QString> mapInfo;
    loadCpuInfo(mapInfo, "lscpu");
    DeviceManager::instance()->setCpuRefreshInfoFromlscpu(mapInfo);
}
