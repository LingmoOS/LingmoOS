/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakeprocessor.h"

#include <QStringList>
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeProcessor::FakeProcessor(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeProcessor::~FakeProcessor()
{
}

int FakeProcessor::number() const
{
    return fakeDevice()->property("number").toInt();
}

int FakeProcessor::maxSpeed() const
{
    return fakeDevice()->property("maxSpeed").toInt();
}

bool FakeProcessor::canChangeFrequency() const
{
    return fakeDevice()->property("canChangeFrequency").toBool();
}

Solid::Processor::InstructionSets FakeProcessor::instructionSets() const
{
    Solid::Processor::InstructionSets result;

    const QStringList extension_list = fakeDevice()->property("instructionSets").toString().split(',');
    for (const QString &extension_str : extension_list) {
        if (extension_str == QLatin1String("mmx")) {
            result |= Solid::Processor::IntelMmx;
        } else if (extension_str == QLatin1String("sse")) {
            result |= Solid::Processor::IntelSse;
        } else if (extension_str == QLatin1String("sse2")) {
            result |= Solid::Processor::IntelSse2;
        } else if (extension_str == QLatin1String("sse3")) {
            result |= Solid::Processor::IntelSse3;
        } else if (extension_str == QLatin1String("sse4")) {
            result |= Solid::Processor::IntelSse4;
        } else if (extension_str == QLatin1String("3dnow")) {
            result |= Solid::Processor::Amd3DNow;
        } else if (extension_str == QLatin1String("altivec")) {
            result |= Solid::Processor::AltiVec;
        }
    }

    return result;
}

#include "moc_fakeprocessor.cpp"
