/*
    SPDX-FileCopyrightText: 2018 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "dadictionary_p.h"

using namespace Solid::Backends::IOKit;

DADictionary::DADictionary(const IOKitDevice *device)
    : device(device)
    , daSession(DASessionCreate(kCFAllocatorDefault))
    , daDict(nullptr)
{
    if (daSession) {
        const QString devName = device->property(QStringLiteral("BSD Name")).toString();
        daRef = DADiskCreateFromBSDName(kCFAllocatorDefault, daSession, devName.toStdString().c_str());
    } else {
        daRef = nullptr;
    }
}

DADictionary::~DADictionary()
{
    releaseDict();
    if (daRef) {
        CFRelease(daRef);
        daRef = nullptr;
    }
    if (daSession) {
        CFRelease(daSession);
        daSession = nullptr;
    }
}

bool DADictionary::getDict()
{
    // daDict may cache the latest disk description dict;
    // we will refresh it now.
    releaseDict();
    if (daRef) {
        daDict = DADiskCopyDescription(daRef);
    }
    return daDict != nullptr;
}

void DADictionary::releaseDict()
{
    if (daDict) {
        CFRelease(daDict);
        daDict = nullptr;
    }
}

const QString DADictionary::stringForKey(const CFStringRef key)
{
    QString ret;
    if (getDict()) {
        ret = QString::fromCFString((const CFStringRef)CFDictionaryGetValue(daDict, key));
        releaseDict();
    }
    return ret;
}

CFURLRef DADictionary::cfUrLRefForKey(const CFStringRef key)
{
    CFURLRef ret = nullptr;
    if (getDict()) {
        ret = (const CFURLRef)CFDictionaryGetValue(daDict, key);
    }
    // we cannot release the dictionary here, or else we'd need to
    // copy the CFURLRef and oblige our caller to release the return value.
    return ret;
}

bool DADictionary::boolForKey(const CFStringRef key, bool &value)
{
    if (getDict()) {
        const CFBooleanRef boolRef = (const CFBooleanRef)CFDictionaryGetValue(daDict, key);
        if (boolRef) {
            value = CFBooleanGetValue(boolRef);
        }
        releaseDict();
        return boolRef != nullptr;
    }
    return false;
}
