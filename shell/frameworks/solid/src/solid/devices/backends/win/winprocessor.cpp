/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "winprocessor.h"

#include <solid/devices/backends/shared/cpufeatures.h>

#include <QSettings>

using namespace Solid::Backends::Win;

typedef BOOL(WINAPI *GLPI_fn)(SYSTEM_LOGICAL_PROCESSOR_INFORMATION *Buffer, DWORD *ReturnLength);
GLPI_fn pGetLogicalProcessorInformation = (GLPI_fn)GetProcAddress(LoadLibraryA("kernel32.dll"), "GetLogicalProcessorInformation");

WinProcessor::WinProcessor(WinDevice *device)
    : WinInterface(device)
{
    m_number = m_device->udi().mid(m_device->udi().length() - 1).toInt();
}

WinProcessor::~WinProcessor()
{
}

int WinProcessor::number() const
{
    return m_number;
}

int WinProcessor::maxSpeed() const
{
    return updateCache()[m_number].speed;
}

bool WinProcessor::canChangeFrequency() const
{
    // TODO:implement
    return false;
}

Solid::Processor::InstructionSets WinProcessor::instructionSets() const
{
    static Solid::Processor::InstructionSets set = Solid::Backends::Shared::cpuFeatures();
    return set;
}

QSet<QString> WinProcessor::getUdis()
{
    static QSet<QString> out;
    if (out.isEmpty()) {
        const QMap<int, ProcessorInfo> map = updateCache();
        for (const ProcessorInfo &info : map) {
            out << QString("/org/kde/solid/win/cpu/device#%1,cpu#%2").arg(info.id).arg(info.lgicalId);
        }
    }
    return out;
}

DWORD WinProcessor::countSetBits(ULONG_PTR bitMask)
{
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
    DWORD i;

    for (i = 0; i <= LSHIFT; ++i) {
        bitSetCount += ((bitMask & bitTest) ? 1 : 0);
        bitTest /= 2;
    }

    return bitSetCount;
}

const QMap<int, WinProcessor::ProcessorInfo> &WinProcessor::updateCache()
{
    static QMap<int, ProcessorInfo> p;

    if (p.isEmpty()) {
        DWORD size = 0;
        pGetLogicalProcessorInformation(NULL, &size);
        char *buff = new char[size];
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION *info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)buff;
        pGetLogicalProcessorInformation(info, &size);
        size /= sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

        uint processorCoreCount = 0;
        uint logicalProcessorCount = 0;

        for (uint i = 0; i < size; ++i) {
            if (info[i].Relationship == RelationProcessorCore) {
                // A hyperthreaded core supplies more than one logical processor.
                uint old = logicalProcessorCount;
                logicalProcessorCount += countSetBits(info[i].ProcessorMask);
                for (; old < logicalProcessorCount; ++old) {
                    QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\") + QString::number(old),
                                       QSettings::NativeFormat);
                    ProcessorInfo proc;
                    proc.id = processorCoreCount;
                    proc.lgicalId = old;
                    proc.speed = settings.value("~MHz").toInt();
                    proc.vendor = settings.value("VendorIdentifier").toString().trimmed();
                    proc.name = settings.value("ProcessorNameString").toString().trimmed();
                    proc.produuct = settings.value("Identifier").toString().trimmed();

                    p[old] = proc;
                }
                processorCoreCount++;
            }
        }
        delete[] buff;
    }
    return p;
}

#include "moc_winprocessor.cpp"
