/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QProcess>
#include <memory>

class NvidiaSmiProcess : public QObject
{
    Q_OBJECT

public:
    struct GpuData {
        int index = -1;
        uint power = 0;
        uint temperature = 0;
        uint usage = 0;
        uint memoryUsed = 0;
        uint coreFrequency = 0;
        uint memoryFrequency = 0;
    };

    struct GpuQueryResult {
        QString pciPath;
        QString name;
        uint totalMemory = 0;
        uint maxCoreFrequency = 0;
        uint maxMemoryFrequency = 0;
        uint maxTemperature = 0;
        uint maxPower = 0;
    };

    NvidiaSmiProcess();

    bool isSupported() const;

    std::vector<GpuQueryResult> query();

    void ref();
    void unref();

    Q_SIGNAL void dataReceived(const GpuData &data);

    void readStatisticsData(const QString &line);

private:
    struct dmonIndices {
        int gpu = -1;
        int gtemp = -1;
        int power = -1;
        int sm = -1;
        int enc = -1;
        int dec = -1;
        int fb = -1;
        int bar1 = -1;
        int mclk = -1;
        int pclk = -1;
    };

    QString m_smiPath;
    std::vector<GpuQueryResult> m_queryResult;
    std::unique_ptr<QProcess> m_process = nullptr;
    int m_references = 0;
    dmonIndices m_dmonIndices;
};
