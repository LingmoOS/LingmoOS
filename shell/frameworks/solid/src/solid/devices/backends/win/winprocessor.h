/*
    SPDX-FileCopyrightText: 2013 Patrick von Reth <vonreth@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WINPROCESSOR_H
#define WINPROCESSOR_H

#include <solid/devices/ifaces/processor.h>

#include "wininterface.h"
namespace Solid
{
namespace Backends
{
namespace Win
{
class WinProcessor : public WinInterface, public Solid::Ifaces::Processor
{
    friend class WinDevice;
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Processor)
public:
    WinProcessor(WinDevice *device);
    ~WinProcessor();

    virtual int number() const;

    virtual int maxSpeed() const;

    virtual bool canChangeFrequency() const;

    virtual Solid::Processor::InstructionSets instructionSets() const;

    static QSet<QString> getUdis();

private:
    int m_number;

    class ProcessorInfo
    {
    public:
        int lgicalId;
        int id;
        int speed;
        QString vendor;
        QString name;
        QString produuct;
    };

    static DWORD countSetBits(ULONG_PTR bitMask);
    static const QMap<int, WinProcessor::ProcessorInfo> &updateCache();
};
}
}
}

#endif // WINPROCESSOR_H
