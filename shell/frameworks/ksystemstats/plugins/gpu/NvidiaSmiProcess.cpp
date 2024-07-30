/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <davidedmundson@kde.org>
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "NvidiaSmiProcess.h"

#include <QStandardPaths>

using namespace Qt::StringLiterals;

NvidiaSmiProcess::NvidiaSmiProcess()
{
    m_smiPath = QStandardPaths::findExecutable(QStringLiteral("nvidia-smi"));
}

bool NvidiaSmiProcess::isSupported() const
{
    return !m_smiPath.isEmpty();
}

std::vector<NvidiaSmiProcess::GpuQueryResult> NvidiaSmiProcess::query()
{
    if (!isSupported()) {
        return m_queryResult;
    }

    if (!m_queryResult.empty()) {
        return m_queryResult;
    }

    // Read and parse the result of "nvidia-smi query"
    // This seems to be the only way to get certain values like total memory or
    // maximum temperature. Unfortunately the output isn't very easily parseable
    // so we have to do some trickery to parse things.

    QProcess queryProcess;
    queryProcess.setProgram(m_smiPath);
    queryProcess.setArguments({QStringLiteral("--query")});
    queryProcess.start();

    int gpuCounter = 0;
    auto data = m_queryResult.end();

    bool readMemory = false;
    bool readMaxClocks = false;
    bool readMaxPwr = false;

    while (queryProcess.waitForReadyRead()) {
        if (!queryProcess.canReadLine()) {
            continue;
        }

        auto line = queryProcess.readLine();
        if (line.startsWith("GPU ")) {
            // Start of GPU properties block. Ensure we have a new data object
            // to write to.
            data = m_queryResult.emplace(m_queryResult.end());
            // nvidia-smi has to much zeros compared to linux, remove line break
            data->pciPath = line.mid(strlen("GPU 0000")).chopped(1).toLower();
            gpuCounter++;
        }

        if ((readMemory || readMaxClocks) && !line.startsWith("        ")) {
            // Memory/clock information does not have a unique prefix but should
            // be indented more than their "headers". So if the indentation is
            // less, we are no longer in an info block and should treat it as
            // such.
            readMemory = false;
            readMaxClocks = false;
        }

        if (line.startsWith("    Product Name")) {
            data->name = line.mid(line.indexOf(':') + 1).trimmed();
        }

        if (line.startsWith("    FB Memory Usage") || line.startsWith("    BAR1 Memory Usage")) {
            readMemory = true;
        }

        if (line.startsWith("    Max Clocks")) {
            readMaxClocks = true;
        }

        if (line.startsWith("    Power Readings")) {
            readMaxPwr = true;
        }

        if (line.startsWith("        Total") && readMemory) {
            data->totalMemory += std::atoi(line.mid(line.indexOf(':') + 1));
        }

        if (line.startsWith("        GPU Shutdown Temp")) {
            data->maxTemperature = std::atoi(line.mid(line.indexOf(':') + 1));
        }

        if (line.startsWith("        Graphics") && readMaxClocks) {
            data->maxCoreFrequency = std::atoi(line.mid(line.indexOf(':') + 1));
        }

        if (line.startsWith("        Memory") && readMaxClocks) {
            data->maxMemoryFrequency = std::atoi(line.mid(line.indexOf(':') + 1));
        }

        if (line.startsWith("        Power Limit") && readMaxPwr) {
            data->maxPower = std::atoi(line.mid(line.indexOf(':') + 1));
        }
    }

    return m_queryResult;
}

void NvidiaSmiProcess::ref()
{
    if (!isSupported()) {
        return;
    }

    m_references++;

    if (m_process) {
        return;
    }

    m_process = std::make_unique<QProcess>();
    m_process->setProgram(m_smiPath);
    m_process->setArguments({
        QStringLiteral("dmon"), // Monitor
        QStringLiteral("-d"),
        QStringLiteral("2"), // 2 seconds delay, to match daemon update rate
        QStringLiteral("-s"),
        QStringLiteral("pucm") // Include all relevant statistics
    });
    connect(m_process.get(), &QProcess::readyReadStandardOutput, this, [this] {
        while (m_process->canReadLine()) {
            const QString line = m_process->readLine();
            readStatisticsData(line);
        }
    });
    m_process->start();
}

void NvidiaSmiProcess::unref()
{
    if (!isSupported()) {
        return;
    }

    m_references--;

    if (!m_process || m_references > 0) {
        return;
    }

    m_process->terminate();
    m_process->waitForFinished();
    m_process.reset();
}

void NvidiaSmiProcess::readStatisticsData(const QString &line)
{
    QList<QStringView> parts = QStringView(line).trimmed().split(QLatin1Char(' '), Qt::SkipEmptyParts);

    // discover index of fields in the header format is something like
    //# gpu   pwr gtemp mtemp    sm   mem   enc   dec  mclk  pclk    fb  bar1
    // # Idx     W     C     C     %     %     %     %   MHz   MHz    MB    MB
    // 0     25     29      -     1      1      0      0   4006   1506    891     22
    if (line.startsWith(QLatin1Char('#'))) {
        if (m_dmonIndices.gpu == -1) {
            // Remove First part because of leading '# ';
            parts.removeFirst();
            m_dmonIndices.gpu = parts.indexOf("gpu"_L1);
            m_dmonIndices.power = parts.indexOf("pwr"_L1);
            m_dmonIndices.gtemp = parts.indexOf("gtemp"_L1);
            m_dmonIndices.sm = parts.indexOf("sm"_L1);
            m_dmonIndices.enc = parts.indexOf("enc"_L1);
            m_dmonIndices.dec = parts.indexOf("dec"_L1);
            m_dmonIndices.fb = parts.indexOf("fb"_L1);
            m_dmonIndices.bar1 = parts.indexOf("bar1"_L1);
            m_dmonIndices.mclk = parts.indexOf("mclk"_L1);
            m_dmonIndices.pclk = parts.indexOf("pclk"_L1);
        }
        return;
    }

    auto readDataIfFound = [&parts] (int index) {
        return index >= 0 ? parts[index].toUInt() : 0;
    };

    GpuData data;
    data.index = readDataIfFound(m_dmonIndices.gpu);
    data.power = readDataIfFound(m_dmonIndices.power);
    data.temperature = readDataIfFound(m_dmonIndices.gtemp);

    // GPU usage equals "SM" usage + "ENC" usage + "DEC" usage
    data.usage = readDataIfFound(m_dmonIndices.sm) + readDataIfFound(m_dmonIndices.enc) + readDataIfFound(m_dmonIndices.dec);

    // Total memory used equals "FB" usage + "BAR1" usage
    data.memoryUsed = readDataIfFound(m_dmonIndices.fb) + readDataIfFound(m_dmonIndices.bar1);

    data.memoryFrequency = readDataIfFound(m_dmonIndices.mclk);
    data.coreFrequency = readDataIfFound(m_dmonIndices.pclk);

    Q_EMIT dataReceived(data);
}

#include "moc_NvidiaSmiProcess.cpp"
