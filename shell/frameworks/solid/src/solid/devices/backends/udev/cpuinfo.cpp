/*
    SPDX-FileCopyrightText: 2010 Alex Merry <alex.merry@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "cpuinfo.h"
#include "cpuinfo_arm.h"

#include <QFile>
#include <QRegularExpression>
#include <QStringList>

namespace Solid
{
namespace Backends
{
namespace UDev
{
class CpuInfo
{
public:
    CpuInfo();
    QString extractCpuInfoLine(int processorNumber, const QString &regExpStr);
    QString extractInfoLine(const QString &regExpStr);

private:
    QStringList cpuInfo;
};

QString extractCpuVendor(int processorNumber)
{
    CpuInfo info;
    QString vendor;

#ifndef BUILDING_FOR_ARM_TARGET
    vendor = info.extractCpuInfoLine(processorNumber, "vendor_id\\s+:\\s+(\\S.+)");
    if (vendor.isEmpty()) {
        vendor = info.extractInfoLine("Hardware\\s+:\\s+(\\S.+)");
    }
#else
    // ARM ? "CPU implementer : 0x41"
    vendor = info.extractCpuInfoLine(processorNumber, "CPU implementer\\s+:\\s+(\\S.+)");
    bool ok = false;
    const int vendorId = vendor.toInt(&ok, 16);
    if (ok) {
        const ArmCpuImplementer *impl = findArmCpuImplementer(vendorId);
        if (impl) {
            vendor = QString::fromUtf8(impl->name);
        }
    }
#endif

    return vendor;
}

QString extractCpuModel(int processorNumber)
{
    CpuInfo info;
    QString model;

#ifndef BUILDING_FOR_ARM_TARGET
    model = info.extractCpuInfoLine(processorNumber, "model name\\s+:\\s+(\\S.+)");
    if (model.isEmpty()) {
        model = info.extractInfoLine("Processor\\s+:\\s+(\\S.+)");
    }

    // for ppc64, extract from "cpu:" line
    if (model.isEmpty()) {
        model = info.extractInfoLine("cpu\\s+:\\s+(\\S.+)");
    }
#else
    // ARM? "CPU part        : 0xd03"
    const QString vendor = info.extractCpuInfoLine(processorNumber, "CPU implementer\\s+:\\s+(\\S.+)");
    model = info.extractCpuInfoLine(processorNumber, "CPU part\\s+:\\s+(\\S.+)");
    if (!model.isEmpty() && !vendor.isEmpty()) {
        bool vendorOk = false, modelOk = false;
        const int vendorId = vendor.toInt(&vendorOk, 16);
        const int modelId = model.toInt(&modelOk, 16);
        if (vendorOk && modelOk) {
            model = findArmCpuModel(vendorId, modelId);
        }
    }
#endif

    return model;
}

int extractCurrentCpuSpeed(int processorNumber)
{
    CpuInfo info;
    int speed = info.extractCpuInfoLine(processorNumber, "cpu MHz\\s+:\\s+(\\d+).*").toInt();
    return speed;
}

CpuInfo::CpuInfo()
{
    QFile cpuInfoFile("/proc/cpuinfo");
    if (!cpuInfoFile.open(QIODevice::ReadOnly)) {
        return;
    }
    cpuInfo = QString(cpuInfoFile.readAll()).split('\n', Qt::SkipEmptyParts);
}

QString CpuInfo::extractCpuInfoLine(int processorNumber, const QString &regExpStr)
{
    const QRegularExpression processorRegExp(QRegularExpression::anchoredPattern(QStringLiteral("processor\\s+:\\s+(\\d+)")));
    const QRegularExpression regExp(QRegularExpression::anchoredPattern(regExpStr));

    int line = 0;
    while (line < cpuInfo.size()) {
        QRegularExpressionMatch match;
        if ((match = processorRegExp.match(cpuInfo.at(line))).hasMatch()) {
            int recordProcNum = match.captured(1).toInt();
            if (recordProcNum == processorNumber) {
                ++line;
                while (line < cpuInfo.size()) {
                    if ((match = regExp.match(cpuInfo.at(line))).hasMatch()) {
                        return match.captured(1);
                    }
                    ++line;
                }
            }
        }
        ++line;
    }

    return QString();
}

QString CpuInfo::extractInfoLine(const QString &regExpStr)
{
    const QRegularExpression regExp(QRegularExpression::anchoredPattern(regExpStr));

    for (const QString &line : std::as_const(cpuInfo)) {
        QRegularExpressionMatch match = regExp.match(line);
        if (match.hasMatch()) {
            return match.captured(1);
        }
    }
    return QString();
}

}
}
}
