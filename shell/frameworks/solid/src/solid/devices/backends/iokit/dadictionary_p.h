/*
    SPDX-FileCopyrightText: 2018 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_DADICTIONARY_P_H
#define SOLID_BACKENDS_IOKIT_DADICTIONARY_P_H

#include <QString>

#include "iokitdevice.h"

#include <CoreFoundation/CoreFoundation.h>
#include <DiskArbitration/DiskArbitration.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class DADictionary
{
public:
    DADictionary(const IOKitDevice *device);
    virtual ~DADictionary();

    /**
     * get a fresh copy of the DA disk description dict;
     * the result is stored in daRef (after releasing any
     * dict it may currently point to).
     */
    bool getDict();
    /**
     * release the DA disk description dict and reset daRef.
     */
    void releaseDict();

    /**
     * fetch the value of @p key as a string, from the current
     * disk description (calls getDict() and releaseDict()).
     */
    const QString stringForKey(const CFStringRef key);
    /**
     * fetch the value of @p key as a CFURLRef, from the current
     * disk description. Calls getDict() but not releaseDict().
     *The contents of the CFURLRef must be retrieved before
     * calling releaseDict() (and thus getDict()).
     */
    CFURLRef cfUrLRefForKey(const CFStringRef key);
    /**
     * fetch the value of @p key as a boolean, from the current
     * disk description (calls getDict() and releaseDict()).
     */
    bool boolForKey(const CFStringRef key, bool &value);

    const IOKitDevice *device;
    DASessionRef daSession;
    DADiskRef daRef;
    CFDictionaryRef daDict;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_DADICTIONARY_P_H
